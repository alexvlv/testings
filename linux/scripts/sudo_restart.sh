#! /bin/sh

# $Id$

echo "Curent UID: $(id -u)"
echo [$0]
SCRIPTDIR="$(dirname "$(readlink -f "$0")")"
echo [$SCRIPTDIR]

#[ "$EUID" -ne 0 ] && { echo "ERROR: You must be root!"; exit 1; }
#[ $(id -u) -ne 0 ] && { echo "ERROR: You must be root!"; exit 1; } 
[ $(id -u) -ne 0 ] && { sudo "$0"; exit 1; } 
echo "Continue as $(id -u) !"
