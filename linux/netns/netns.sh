#!/usr/bin/env bash

# GIT Rev.: $Format:%cd %cn %h %D$

set -e

# Physical interface configuration.
IF_NAME="router"
IP_ADDR="192.168.35.101/24"
GW="192.168.35.100"
DEFAULT_GW="$GW"
NSNAME="net"
VPN_SERVERS="buh fmsk imsk"
DNS_SERVERS="9.9.9.11 94.140.14.14"

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
	} || true
}

ns_require_present() {
	local ns="$1"

	ns_exists "$ns" || {
		echo "Namespace does not exist: $ns" >&2
		return 1
	}
}

# Terminate all processes in the namespace.
ns_kill_processes() {
	local ns_name="$1"
	local pids

	pids=$(ip netns pids "$ns_name" | tr '\n' ' ')
	[ -n "$pids" ] && {
		echo "Terminating $pids ...";
		kill $pids;
		sleep 1;
	}
	pids=$(ip netns pids "$ns_name" | tr '\n' ' ')
	[ -n "$pids" ] && { echo "Killing $pids ..."; sleep 1; kill -KILL $pids; }
	return 0
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
		echo "VPN $wg_if down ..."

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
# Namespace operations
# ---------------------------------------------------------------------------

netns_up() {
	local ns="$1"
	local iface="$2"
	local wg_config="$3"

	ns_require_absent "$ns"

	echo "Creating namespace: $ns"

	ip link show "$iface" >/dev/null 2>&1 || {
		echo "Interface does not exist: $iface" >&2
		return 1
	}

	ip netns add "$ns"
	ip netns exec "$ns" ip link set lo up

	ip link set "$iface" netns "$ns"

	ip netns exec "$ns" ip addr flush dev "$iface"
	ip netns exec "$ns" ip addr add "$IP_ADDR" dev "$iface"
	ip netns exec "$ns" ip link set "$iface" up

	ns_dns_setup "$ns"

	vpn_resolve_servers
	local ip
	for ip in "${VPN_SERVER_IPS[@]}"; do
		ip netns exec "$ns" \
			ip route add "$ip/32" via "$GW" dev "$iface"
	done

	[ -n "$wg_config" ] || {
		[ -n "$DEFAULT_GW" ] && \
		echo "Setting default gateway: $DEFAULT_GW ..." && \
		ip netns exec "$ns" ip route add default via "$DEFAULT_GW" dev "$iface"
	}

	ip netns exec "$ns" ip addr show
	ip netns exec "$ns" ip route show
	ip netns list

	[ -n "$wg_config" ] || return 0

	echo "Starting VPN: $wg_config"
	vpn_start "$ns" "$wg_config"
}

netns_down() {
	local ns="$1"

	ns_require_present "$ns"

	echo "Destroying namespace: $ns"

	vpn_stop "$ns"
	ns_kill_processes "$ns"

	ip netns del "$ns"
	echo "Done!"
	ip netns list

	#nmcli device set "$iface" managed yes 2>/dev/null || true
	#nmcli device connect "$iface" 2>/dev/null || truen.cs
}


# ---------------------------------------------------------------------------
# Command line
# ---------------------------------------------------------------------------

[ "$#" -eq 0 ] && {
	#ip netns identify $$
	ip netns list
	exit 0
}

[ "$#" -ge 1 ] || {
	exit 1
}

case "$1" in
	up)
		#netns_up "$1" "${3:-}" "${4:-}"
		netns_up "$NSNAME" "$IF_NAME" ${2:-}
		;;

	down)
		netns_down "$NSNAME"
		;;

	*)
		exit 1
		;;
esac
