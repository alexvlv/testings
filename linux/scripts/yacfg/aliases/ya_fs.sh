. ${ALIAS_DIR}/terminal.sh

YAFS_LNK=~/.yafs
#[ -L ${YAFS_LNK} ] && YAFS_DIR=$(readlink -s -e ${YAFS_LNK})
#[ -z ${YAFS_DIR} ] && YAFS_DIR=./
#YACFG_DIR=${YAFS_DIR}/cfg

#Read only!
alias ya.sr='(tt YAND; [ -L ${YAFS_LNK} ] && YAFS_DIR=$(readlink -s -e ${YAFS_LNK}); [ -z ${YAFS_DIR} ] && YAFS_DIR=./; cd ${YAFS_DIR}/yacfg && yandex-disk -c cfg --read-only start && sleep 1 && watch yandex-disk -c cfg status)'
alias ya.s='(tt YAND; [ -L ${YAFS_LNK} ] && YAFS_DIR=$(readlink -s -e ${YAFS_LNK}); [ -z ${YAFS_DIR} ] && YAFS_DIR=./; cd ${YAFS_DIR}/yacfg && yandex-disk -c cfg start && sleep 1 && watch yandex-disk -c cfg status)'
alias ya.o='([ -L ${YAFS_LNK} ] && YAFS_DIR=$(readlink -s -e ${YAFS_LNK}); [ -z ${YAFS_DIR} ] && YAFS_DIR=./; cd ${YAFS_DIR}/yacfg && yandex-disk -c cfg stop)'
alias ya.w='(tt YAND; [ -L ${YAFS_LNK} ] && YAFS_DIR=$(readlink -s -e ${YAFS_LNK}); [ -z ${YAFS_DIR} ] && YAFS_DIR=./; cd ${YAFS_DIR}/yacfg && watch yandex-disk -c cfg status)'
#alias ya.t='(echo [$YACFG_DIR] && cd $YACFG_DIR)'

alias cy='[ -L ${YAFS_LNK} ] && YAFS_DIR=$(readlink -s -e ${YAFS_LNK}); [ ! -z ${YAFS_DIR} ] && cd ${YAFS_DIR}'
