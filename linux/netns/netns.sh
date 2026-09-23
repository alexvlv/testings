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

netns_up() {
	local ns_name="$1"
	local wg_config="$2"

	ip netns exec "$ns_name" true 2>/dev/null && {
		echo "Namespace already exists: $ns_name" >&2
		return 1
	}

	local network_id
	network_id=$(( $(ip netns list | wc -l) + 1 ))

	local ns_net="10.200.$network_id.0"
	local ns_host="10.200.$network_id.1"
	local ns_ip="10.200.$network_id.2"

	echo "Creating namespace: $ns_name"
	echo "Network: $ns_net/30"

	netns_create "$ns_name" "$ns_net"

	[ -n "$wg_config" ] || return 0

	ip netns exec "$ns_name" \
		wg-quick up "/etc/wireguard/$wg_config.conf"
}

netns_down() {
	local ns_name="$1"

	ip netns exec "$ns_name" true 2>/dev/null || {
		echo "Namespace does not exist: $ns_name" >&2
		return 1
	}

	local ns_net
	ns_net=$(ip netns exec "$ns_name" \
		ip -4 addr show "veth-$ns_name" |
		awk '/inet / {sub(/\/.*/, "", $2); split($2, a, "."); print a[1]"."a[2]"."a[3]".0"}')

	netns_destroy "$ns_name" "$ns_net"
}

usage() {
	echo "Usage:"
	echo "  $0 <namespace> <network_id> {up|down}"
	exit 1
}

case "$2" in
	up)
		[ "$#" -le 3 ] || usage
		netns_up "$1" "$3"
		;;
	down)
		[ "$#" -eq 2 ] || usage
		netns_down "$1"
		;;
	*)
		usage
		;;
esac
