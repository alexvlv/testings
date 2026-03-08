#!/bin/bash

# GIT Rev.: $Format:%cd %cn %h %D$

set -e

DST=/var/run/user/$UID/psw

[ -s "$DST" ] && {
  echo "Already stored."
  [[ "$1" == "c"* ]] && { 
    rm "$DST" &&  echo Cleared!
  }
  exit 0
}


read -r -sp "Enter your password: " password
echo # Add a newline after silent input
[ -z "$password" ] && {
  echo "ERROR: empty password!"
  exit 1
}
echo "$password" >"$DST" || exit 1

printf '\033[?1049h'
printf "Password: \"%s\"\n" "$password"
read -p "<Enter to hide>"
printf '\033[?1049l'

# \033[K = erase line. 
#printf "\r\033[K"
