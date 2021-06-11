#!/bin/bash
#
# build.sh for opensbi
# author: liush@allwinnertech.com
# 

set -e

TOP_DIR=$(cd $(dirname $0); pwd)
VENDOR=NULL
VENDOR_IP=NULL
BUILD_CHIP=NULL
OPENSBI_TARGET_DIR=NULL
TOOLCHAIN_DIR=./../tools/toolchain/
#RISCV_TOOLCHAIN=riscv64-linux-x86_64-20200528.tar.xz
#RISCV_TOOLCHAIN_DIR=riscv64-linux-x86_64-20200528/bin/
RISCV_TOOLCHAIN=riscv64-glibc-gcc-thead_20200702.tar.xz
RISCV_TOOLCHAIN_DIR=riscv64-glibc-gcc-thead_20200702/bin/
RISCV_TOOLCHAIN_PREFIX=riscv64-unknown-linux-gnu-
BUILDCONFIG=../../../.buildconfig

show_help()
{
	printf "\nbuild.sh - opensbi build scritps\n"
	echo "Valid Options:"
	echo "  -h  Show help message"
	echo "  -p <chip> chip, e.g. sun20iw1p1."
	echo "example:"
	echo "./build.sh -p sun20iw1p1"
	printf "\n\n"
}

prepare_toolchain()
{
	local toolchain_archive_riscv=${TOOLCHAIN_DIR}${RISCV_TOOLCHAIN};
	local tooldir_riscv=${TOOLCHAIN_DIR}${RISCV_TOOLCHAIN_DIR};

	if [ ! -d ${tooldir_riscv} ];then
		echo "Prepare toolchain ..."
		echo ${toolchain_archive_riscv}
		#tar -xvjf ${toolchain_archive_riscv} -C ${TOOLCHAIN_DIR}
		tar -Jxvf ${toolchain_archive_riscv} -C ${TOOLCHAIN_DIR}
	fi
}

if [ -f ${BUILDCONFIG} ];then
	echo ${BUILDCONFIG}
	BUILD_CHIP=$(cat ${BUILDCONFIG} | grep "LICHEE_CHIP\>" | awk -F'=' '{ print $2 }')
	OPENSBI_TARGET_DIR=$(cat ${BUILDCONFIG} | grep "LICHEE_BRANDY_OUT_DIR" | awk -F'=' '{ print $2 }')
elif [ "x$TARGET_BUILD_VARIANT" = "xtina" ];then
	BUILD_CHIP=$TARGET_CHIP
	OPENSBI_TARGET_DIR=$TOP_DIR/../../../device/config/chips/$TARGET_PLATFORM/bin
else
	echo "buidconfig file is null." 
fi

if [ -f ${BUILD_CHIP}.config ];then
	VENDOR=$(cat ${BUILD_CHIP}.config | grep "vendor" | awk -F'=' '{ print $2 }')
	VENDOR_IP=$(cat ${BUILD_CHIP}.config | grep "ip" | awk -F'=' '{ print $2 }')
else
	echo "build opensbi: can not find config file." && return 1
fi

if [ "x${VENDOR}" != "x" ] && [ "x${VENDOR_IP}" != "x" ];then
	echo vendor/ip:${VENDOR}/${VENDOR_IP}
	make distclean
	prepare_toolchain
	make PLATFORM=${VENDOR}/${VENDOR_IP} CROSS_COMPILE=${TOOLCHAIN_DIR}${RISCV_TOOLCHAIN_DIR}${RISCV_TOOLCHAIN_PREFIX} SUNXI_CHIP=${BUILD_CHIP} PLATFORM_RISCV_ISA=rv64gcxthead
	echo "build ok"
	echo "copy bin"
	OPENSBI_FIRMWARE_BIN=fw_jump.bin
	OPENSBI_FIRMWARE_BIN_PATH=build/platform/${VENDOR}/${VENDOR_IP}/firmware
	if [ "x${OPENSBI_TARGET_DIR}" != "xNULL" ];then
		cp  ${OPENSBI_FIRMWARE_BIN_PATH}/${OPENSBI_FIRMWARE_BIN}  ${OPENSBI_TARGET_DIR}/opensbi_${BUILD_CHIP}.bin
	else
		echo "copy opensbi bin failed." && return 1
	fi
else
	echo "build opensbi: can not find vendor or ip." && return 1
fi
exit $?
