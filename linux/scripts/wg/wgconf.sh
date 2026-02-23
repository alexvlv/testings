#! /bin/bash

# GIT Rev.: $Format:%cd %cn %h %D$

b=${PWD##*/}; export number=${b%%_*}; export name=${b#*_}

[ -n "$number" ] || { echo "No number"; exit; }
[ -n "$name" ] || { echo "No name"; exit; }

export num=$((10#$number))

read PrivateKey < privatekey || exit
export PrivateKey
read PublicKey < publickey || exit

#basedir="$(dirname "$(readlink -f "$0")")"
basedir="$(dirname "$0")"

confdir=$basedir/config
[ -d "$confdir" ] || { echo "No confdir"; exit; }

outdir=.out

mkdir -p $outdir

for f in $confdir/*.conf; do
	[ -f "$f" ] || continue
	cfgname=$(basename "$f")
	outname="client_${number}_${name}_${cfgname}"
	echo "# $name $(date +"%d/%m/%Y %H:%M")" > $outdir/$outname
	envsubst < $f >> $outdir/$outname
	echo "=== Start of $cfgname ==="
	cat $outdir/$outname
	echo "=== End of $cfgname ==="
	line=$(grep -o 'Address = 10\.[0-9]\{1,3\}\.0\.\${num}' $f)
	eval "line=\"$line\""
	address=$(printf "%s" "$line" | grep -o '10\.[0-9]\{1,3\}\.0\.[0-9]\{1,3\}')
	srvstr="srv_${number}_${name}_${cfgname}"
	cat > $outdir/$srvstr <<-EOF
		# $name $(date +"%d/%m/%Y %H:%M")
		[Peer]
		PublicKey = $PublicKey
		AllowedIPs = $address/32
	EOF
	echo "=== Start of $srvstr ==="
	cat $outdir/$srvstr
	echo "=== End of $srvstr ==="
done
