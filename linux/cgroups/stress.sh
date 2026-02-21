#!/usr/bin/env bash
# 2026-02-21  cgroup scope diagnostic test (fixed)

set -euo pipefail

UNIT=stress-test

echo "[*] Starting stress-ng via systemd-run..."

systemd-run --scope --unit="$UNIT" \
	-p CPUQuota=100% \
	stress-ng --cpu 4 --timeout 20s \
	> /tmp/${UNIT}.log 2>&1 &

# даём systemd создать scope
sleep 1

UNIT_FULL="${UNIT}.scope"

echo
echo "[*] Unit: $UNIT_FULL"

echo
echo "===== systemctl status ====="
systemctl status "$UNIT_FULL" --no-pager

echo
echo "===== unit properties ====="
systemctl show "$UNIT_FULL" \
	-p ControlGroup \
	-p CPUQuotaPerSecUSec \
	-p CPUAccounting

CG_PATH=$(systemctl show "$UNIT_FULL" -p ControlGroup --value)
CG_FULL="/sys/fs/cgroup$CG_PATH"

echo
echo "[*] CGroup path: $CG_PATH"

echo
echo "===== cpu.max ====="
cat "$CG_FULL/cpu.max"

echo
echo "===== cpu.stat ====="
cat "$CG_FULL/cpu.stat"

echo
echo "===== processes in cgroup ====="
cat "$CG_FULL/cgroup.procs"

echo
echo "[*] Waiting for stress-ng to finish..."
wait
echo "[*] Done."
