#!/bin/sh

SDK_DIR=`readlink .sdk`
SRC_LNK=.src

if [ -L ${SRC_LNK} ]; then
	SRC_DIR=`readlink ${SRC_LNK}`
	if [ ! -d ${SRC_DIR} ] || [ ${SRC_DIR} = ${PWD} ] || [ ! -s ${SRC_DIR}/CMakeLists.txt ]; then
		echo Source [${SRC_DIR}] not configured!
		exit
	fi
	BUILD_DIR=${PWD}
else
	SRC_DIR=${PWD}
	BUILD_DIR=${SRC_DIR}/.build
	[ -d ${BUILD_DIR} ] || mkdir -p ${BUILD_DIR}
	SDK_DIR_=`readlink ${BUILD_DIR}/.sdk`
	[ ! -z ${SDK_DIR_} ] && SDK_DIR=${SDK_DIR_}
fi

echo "Source directory: ${SRC_DIR}"
echo "Build  directory: ${BUILD_DIR}"
echo "SDK directory:    ${SDK_DIR}"

BUILD_LOG=${BUILD_DIR}/build.log
CMAKE_LOG=${BUILD_DIR}/cmake.log

if [ ! -z ${SDK_DIR} ] && [ -s ${SDK_DIR}/environment-setup-armv8a-poky-linux ]; then
	. ${SDK_DIR}/environment-setup-armv8a-poky-linux
else
	echo ERROR: SDK not configured!
	exit 
fi

configure () {
	#cmake --version | tee ${BUILD_DIR}/cmake.log
	cmake -S ${SRC_DIR} -B ${BUILD_DIR} #| tee ${BUILD_DIR}/cmake.log
}

build () {
	BUILD_START=$(date +"%d/%m/%Y %H:%M")
	echo "Build [$1] start at $BUILD_START"  #| tee $BUILD_LOG
	echo "CROSS_COMPILE: ${CROSS_COMPILE}" #| tee -a $BUILD_LOG

	#ionice -c 3 nice -n19 make -C ${BUILD_DIR} -j$(($(nproc)+1)) --no-print-directory $1 #2>&1 | tee -a $BUILD_LOG
	ionice -c 3 nice -n19  cmake --build ${BUILD_DIR} -j$(($(nproc)+1)) --target $1
	BUILD_END=$(date +"%d/%m/%Y %H:%M")
	echo
	echo "Build [$1] start at $BUILD_START" #| tee -a $BUILD_LOG
	echo "Build [$1] done  at $BUILD_END" #| tee -a $BUILD_LOG
}

case $1 in
	c) configure; exit ;;
	cmake) configure; exit ;;
	a) TARGET=all ;;
	g) TARGET=git ;;
	v) TARGET=vcmake ;;
	r) TARGET=recmake ;;
	cl) TARGET=clean ;;
	dcl) TARGET=distclean ;;
	*) TARGET=$1
esac

[ ! -z ${TARGET} ] || TARGET=all

[ -s ${BUILD_DIR}/Makefile ] || {
	configure
}
build $TARGET
