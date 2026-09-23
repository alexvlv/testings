#!/usr/bin/env bash

# GIT Rev.: $Format:%cd %cn %h %D$

set -e

VPN_SERVERS="buh fmsk imsk"

[ "$(id -u)" -ne 0 ] && {
	echo "Restarting script as root ..."
	sudo "$0" "$@"
	exit $?
}

# sudo netns_create inet 10.200.0.0
netns_create() {
	local ns_name="$1"

ip netns exec "$ns_name" true 2>/dev/null && {
	echo "Namespace already exists: $ns_name" >&2
	return 1
}

	local ns_net="$2"
	local ns_host="${ns_net%.*}.1"
	local ns_ip="${ns_net%.*}.2"

	local VPN_SERVER_IPS
	for server in $VPN_SERVERS; do
		ip=$(getent ahostsv4 "$server" | awk 'NR==1 {print $1}')
		[ -n "$ip" ] || {
			echo "Cannot resolve VPN server: $server" >&2
			exit 1
		}
		VPN_SERVER_IPS="$VPN_SERVER_IPS $ip"
	done

	ip netns add "$ns_name"

	ip link add "veth-$ns_name" type veth peer name "veth-$ns_name-host"
	ip link set "veth-$ns_name" netns "$ns_name"

	ip addr add "$ns_host/30" dev "veth-$ns_name-host"
	ip link set "veth-$ns_name-host" up

	ip netns exec "$ns_name" ip link set lo up
	ip netns exec "$ns_name" ip addr add "$ns_ip/30" dev "veth-$ns_name"
	ip netns exec "$ns_name" ip link set "veth-$ns_name" up

	for ip in $VPN_SERVER_IPS; do
		ip netns exec "$ns_name" ip route add "$ip/32" via "$ns_host" dev "veth-$ns_name"
	done	
	
	#ip route add "$ns_net/30" dev "veth-$ns_name-host"
	ip route replace "$ns_net/30" dev "veth-$ns_name-host"

	iptables -t nat -A POSTROUTING -s "$ns_net/30" -o wan -j MASQUERADE

	mkdir -p "/etc/netns/$ns_name"
	[ -s "/etc/netns/$ns_name/resolv.conf" ] ||
		printf 'nameserver 1.1.1.1\nnameserver 8.8.8.8\n' > "/etc/netns/$ns_name/resolv.conf"
}


netns_destroy() {
	local ns_name="$1"
	local ns_net="$2"
	local ns_host="${ns_net%.*}.1"

	# Stop WireGuard if it is running in this namespace.
	#ip netns exec "$ns_name" wg-quick down /etc/wireguard/wgbf.conf 2>/dev/null || true
	ip netns exec "$ns_name" wg show interfaces |
	while read -r wg_if; do
		ip netns exec "$ns_name" wg-quick down "/etc/wireguard/$wg_if.conf" 2>/dev/null || true
	done

	# Remove host-side route and NAT.
	ip route del "$ns_net/30" dev "veth-$ns_name-host" 2>/dev/null || true
	iptables -t nat -D POSTROUTING -s "$ns_net/30" -o wan -j MASQUERADE 2>/dev/null || true

	# Removing the namespace also removes its veth side.
	ip netns del "$ns_name" 2>/dev/null || true

	# Remove the per-namespace DNS configuration.
	#rm -rf "/etc/netns/$ns_name"
}

usage() {
	echo "Usage:"
	echo "  $0 <namespace> <network_id> {up|down}"
	echo "  $0 <namespace> <command> [args...]"
	exit 1
}

[ "$#" -ge 2 ] || usage

ns_name="$1"

case "$2" in
	up)
		# netns.sh <ns> up [wg]
		[ "$#" -le 3 ] || usage

		netns_create "$ns_name" "10.200.0.0" || exit $?

		[ "$#" -eq 3 ] &&
			ip netns exec "$ns_name" wg-quick up \
				"/etc/wireguard/$3.conf"
		;;

	down)
		# netns.sh <ns> down
		netns_destroy "$ns_name" "10.200.0.0"
		;;

	*)
		# netns.sh <ns> <network_id> up|down
		case "$3" in
			up|down)
				[ "$#" -ge 3 ] && [ "$#" -le 4 ] || usage

				network_id="$2"
				ns_net="10.200.$network_id.0"

				if [ "$3" = up ]; then
					netns_create "$ns_name" "$ns_net" || exit $?

					[ "$#" -eq 4 ] &&
						ip netns exec "$ns_name" wg-quick up \
							"/etc/wireguard/$4.conf"
				else
					netns_destroy "$ns_name" "$ns_net"
				fi
				;;
			*)
				# Run command inside namespace.
				shift
				ip netns exec "$ns_name" runuser -u "$SUDO_USER" \
					--preserve-environment -- "$@"
				;;
		esac
		;;
esac
#  netns.sh inet  sudo wg-quick up /etc/wireguard/wgbf.conf

