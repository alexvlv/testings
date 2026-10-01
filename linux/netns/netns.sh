#!/usr/bin/env bash

# GIT Rev.: $Format:%cd %cn %h %D$

set -e

VPN_SERVERS="buh fmsk imsk nuker"

# Physical interface configuration.
IF_NAME="wan"
IP_ADDR="192.168.35.101/24"
GW="192.168.35.100"

DNS_SERVERS="1.1.1.1 8.8.8.8"
VETH_PREFIX="veth"
NET_PREFIX="10.200"

[ "$(id -u)" -ne 0 ] && {
	#echo "Restarting script as root ..."
	sudo "$0" "$@"
	exit $?
}


# ---------------------------------------------------------------------------
# Namespace helpers
# ---------------------------------------------------------------------------

ns_exists() {
	ip netns exec "$1" true 2>/dev/null
}

ns_require_absent() {
	local ns="$1"

	ns_exists "$ns" && {
		echo "Namespace already exists: $ns" >&2
		return 1
	} || true;
}

ns_require_present() {
	local ns="$1"

	ns_exists "$ns" || {
		echo "Namespace does not exist: $ns" >&2
		return 1
	}
}

ns_init() {
	local ns="$1"

	ip netns add "$ns"
	ip netns exec "$ns" ip link set lo up
}

ns_dns_setup() {
	local ns="$1"
	local file="/etc/netns/$ns/resolv.conf"

	mkdir -p "/etc/netns/$ns"

	[ -s "$file" ] || {
		for dns in $DNS_SERVERS; do
			printf 'nameserver %s\n' "$dns"
		done > "$file"
	}
}


# ---------------------------------------------------------------------------
# VPN helpers
# ---------------------------------------------------------------------------

vpn_resolve_servers() {
	local server ip
	VPN_SERVER_IPS=()

	for server in $VPN_SERVERS; do
		ip=$(getent ahostsv4 "$server" | awk 'NR==1 {print $1}')

		[ -n "$ip" ] || {
			echo "Cannot resolve VPN server: $server" >&2
			return 1
		}

		VPN_SERVER_IPS+=("$ip")
	done
}

vpn_config() {
	local config="$1"

	WG_CMD="wg-quick"
	WG_SRC="/etc/wireguard/$config.conf"

	case "$config" in
		a*)
			WG_CMD="awg-quick"
			WG_SRC="/etc/amnezia/amneziawg/$config.conf"
			;;
	esac

	[ -f "$WG_SRC" ] || {
		echo "VPN config not found: $WG_SRC" >&2
		return 1
	}
}

vpn_start() {
	local ns="$1"
	local config="$2"
	local tmp_dir="/run/netns-$ns"
	local tmp="$tmp_dir/$config.conf"

	vpn_config "$config"
	mkdir -p "$tmp_dir"

	umask 077
	sed '/^[[:space:]]*DNS[[:space:]]*=/d' "$WG_SRC" > "$tmp"

	if ip netns exec "$ns" "$WG_CMD" up "$tmp"; then
		return 0
	else
		local ret=$?
		rm -f "$tmp"
		return "$ret"
	fi
}

vpn_stop() {
	local ns="$1"
	local wg_if wg_cmd wg_src tmp
	local tmp_dir="/run/netns-$ns"

	while read -r wg_if; do
		[ -n "$wg_if" ] || continue

		wg_cmd="wg-quick"
		wg_src="/etc/wireguard/$wg_if.conf"

		case "$wg_if" in
			a*)
				wg_cmd="awg-quick"
				wg_src="/etc/amnezia/amneziawg/$wg_if.conf"
				;;
		esac

		tmp="$tmp_dir/$wg_if.conf"
		[ ! -f "$tmp" ] || wg_src="$tmp"

		ip netns exec "$ns" \
			"$wg_cmd" down "$wg_src" 2>/dev/null || true
	done < <(ip netns exec "$ns" wg show interfaces)

	rm -rf "$tmp_dir"
}


# ---------------------------------------------------------------------------
# Network setup: veth + NAT
# ---------------------------------------------------------------------------

net_veth_create() {
	local ns="$1"
	local net="$2"
	local host="${net%.*}.1"
	local addr="${net%.*}.2"
	local veth="$VETH_PREFIX-$ns"
	local peer="$veth-host"

	ip link add "$veth" type veth peer name "$peer"
	ip link set "$veth" netns "$ns"

	ip addr add "$host/30" dev "$peer"
	ip link set "$peer" up

	ip netns exec "$ns" ip addr add "$addr/30" dev "$veth"
	ip netns exec "$ns" ip link set "$veth" up

	ip route replace "$net/30" dev "$peer"

	iptables -t nat -A POSTROUTING \
		-s "$net/30" -o "$IF_NAME" -j MASQUERADE

	vpn_resolve_servers

	local ip
	for ip in "${VPN_SERVER_IPS[@]}"; do
		ip netns exec "$ns" \
			ip route add "$ip/32" via "$host" dev "$veth"
	done
}

net_veth_destroy() {
	local ns="$1"
	local net="$2"
	local peer="$VETH_PREFIX-$ns-host"

	[ -n "$net" ] || return 0

	ip route del "$net/30" dev "$peer" 2>/dev/null || true

	iptables -t nat -D POSTROUTING \
		-s "$net/30" -o "$IF_NAME" -j MASQUERADE 2>/dev/null || true
}


# ---------------------------------------------------------------------------
# Network setup: physical interface
# ---------------------------------------------------------------------------

net_physical_create() {
	local ns="$1"
	local iface="$2"

	ip link show "$iface" >/dev/null 2>&1 || {
		echo "Interface does not exist: $iface" >&2
		return 1
	}

	vpn_resolve_servers

	ip link set "$iface" netns "$ns"

	ip netns exec "$ns" ip addr flush dev "$iface"
	ip netns exec "$ns" ip addr add "$IP_ADDR" dev "$iface"
	ip netns exec "$ns" ip link set "$iface" up

	local ip
	for ip in "${VPN_SERVER_IPS[@]}"; do
		ip netns exec "$ns" \
			ip route add "$ip/32" via "$GW" dev "$iface"
	done

	ip netns exec "$ns" \
		ip route add default via "$GW" dev "$iface"
}

net_physical_destroy() {
	local iface="$1"

	# The interface returns to the main namespace when the
	# namespace is deleted. NetworkManager restores its config.
	ip netns del "$2" 2>/dev/null || true

	nmcli device set "$iface" managed yes 2>/dev/null || true
	nmcli device connect "$iface" 2>/dev/null || true
}


# ---------------------------------------------------------------------------
# Network detection and cleanup
# ---------------------------------------------------------------------------

net_veth_address() {
	local ns="$1"

	ip netns exec "$ns" ip -4 -o addr show "$VETH_PREFIX-$ns" |
		awk '{
			split($4, a, "/")
			split(a[1], b, ".")
			print b[1]"."b[2]"."b[3]".0"
			exit
		}'
}

net_physical_detect() {
	local ns="$1"
	local iface

	# Identify a non-veth interface that is not a WG tunnel.
	local wg_interfaces
	wg_interfaces=$(ip netns exec "$ns" wg show interfaces 2>/dev/null || true)

	while read -r iface; do
		[ -n "$iface" ] || continue
		case " $wg_interfaces " in
			*" $iface "*) continue ;;
		esac

		case "$iface" in
			lo|"$VETH_PREFIX"-*) continue ;;
		esac

		printf '%s\n' "$iface"
		return 0
	done < <(ip netns exec "$ns" ip -o link show |
		awk -F': ' '{sub(/@.*/, "", $2); print $2}')

	return 1
}

net_destroy() {
	local ns="$1"
	local iface="$2"
	local net="$3"

	vpn_stop "$ns"

	if [ -n "$iface" ]; then
		net_physical_destroy "$iface" "$ns"
	else
		net_veth_destroy "$ns" "$net"
		ip netns del "$ns" 2>/dev/null || true
	fi
}


# ---------------------------------------------------------------------------
# Namespace operations
# ---------------------------------------------------------------------------

netns_up() {
	local ns="$1"
	local wg_config="$2"
	local iface="$3"

	ns_require_absent "$ns"

	local network_id
	network_id=$(( $(ip netns list | wc -l) + 1 ))

	local net="$NET_PREFIX.$network_id.0"

	echo "Creating namespace: $ns"

	ns_init "$ns"

	if [ -n "$iface" ]; then
		echo "Physical interface: $iface"
		net_physical_create "$ns" "$iface"
	else
		echo "Network: $net/30"
		net_veth_create "$ns" "$net"
	fi

	ns_dns_setup "$ns"

	[ -n "$wg_config" ] || return 0

	echo "Starting VPN: $wg_config"
	vpn_start "$ns" "$wg_config"
}

netns_down() {
	local ns="$1"

	ns_require_present "$ns"

	local iface=""
	local net=""

	iface=$(net_physical_detect "$ns" || true)

	if [ -z "$iface" ]; then
		net=$(net_veth_address "$ns")
	fi

	echo "Destroying namespace: $ns"
	net_destroy "$ns" "$iface" "$net"
}


# ---------------------------------------------------------------------------
# Command line
# ---------------------------------------------------------------------------

usage() {
	cat <<EOF
Usage:
  $0 <namespace> up [<wgconfig>] [<iface>]
  $0 <namespace> down
  $0

Examples:
  $0 inet up
  $0 inet up wgbf
  $0 inet up wgbf wan
  $0 inet up "" wan
  $0 inet down

Without <iface>, a veth/NAT network is created.
With <iface>, the physical interface is moved into the namespace.
EOF
}

[ "$#" -eq 0 ] && {
	#ip netns identify $$
	ip netns list
	exit 0
}

[ "$#" -ge 2 ] || {
	usage
	exit 1
}

case "$2" in
	up)
		[ "$#" -le 4 ] || {
			usage
			exit 1
		}
		netns_up "$1" "${3:-}" "${4:-}"
		;;

	down)
		[ "$#" -eq 2 ] || {
			usage
			exit 1
		}
		netns_down "$1"
		;;

	*)
		usage
		exit 1
		;;
esac
