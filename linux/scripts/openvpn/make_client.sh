#!/bin/sh

# GIT Rev.: $Format:%cd %cn %h %D$

name=${PWD##*/}
[ "$name" != "" ] || { echo "No client name"; exit; }

echo "Generating config for \"$name\" ..."

basedir="$(dirname "$0")"

template="$basedir/template.ovpn"
[ -s "$template" ] || { echo "No template"; exit; }

ca_crt_file="$basedir/ca.crt"
export ca_crt=$(cat "$ca_crt_file" 2>/dev/null) || { echo "No ca.crt"; exit; }
tls_file="$basedir/tls-auth.key"
export tls_auth=$(cat "$tls_file" 2>/dev/null) || { echo "No tls-auth.key"; exit; }



export client_crt=$(cat *.crt 2>/dev/null)
[ -n "$client_crt" ] || { 
	echo "No client .crt "; 
	echo  "cd ~/easy-rsa && ./easyrsa build-client-full $name nopass"
	exit; 
}
export client_key=$(cat *.key 2>/dev/null)
[ -n "$client_key" ] || { echo "No client .key "; exit; }

outname="${name}.ovpn"
echo "# $name generated $(date +"%d/%m/%Y %H:%M") by $USER on $(hostname)" > $outname

envsubst < $template >> $outname
cat $outname

# cd ~/easy-rsa/ && ./easyrsa build-client-full client1 nopass
# Output files:
# Client certificate: pki/issued/client1.crt
# Client key: pki/private/client1.key
