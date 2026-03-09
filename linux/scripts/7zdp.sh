#!/bin/bash

# GIT Rev.: $Format:%cd %cn %h %D$
``
[ "$1" != "" ] && outdir="$1"|| outdir=.

psw_file="$XDG_RUNTIME_DIR/psw"
[ -s "$psw_file" ] || {
  echo "Password must be unlocked first!"
  psw-unlock
  [ -s "$psw_file" ] || exit 1
}
psw=$(<$psw_file)

date=$(/bin/date +%y%m%d-%H%M)
name=$(basename "${PWD}")
ext="tar.7z"

fname=${name}-${date}.${ext}
outfname=${outdir}/${fname}

bytes=$(du -sb ./ | awk '{print $1}')
log=${outdir}/.${fname}.log

echo "Target: $outfname"

7z a "$outfname" -p${psw} ./
