#!/bin/bash
export TARGET_PLATFORM=X86
export SRC_DIR=$PWD
export JAVA_HOME=/usr/lib/jvm/java-6-openjdk-i386/jre
export PATH=$PATH:$JAVA_HOME/bin
export CLASSPATH=.:$JAVA_HOME/lib/dt.jar:$JAVA_HOME/lib/tools.jar
export TARGET_HOST=
export CROSS_COMPILE=
export CC=gcc
export CCLD=ld
export LD=ld
export CP=cp
export BSP_DIR=/
export LTIB_DIR=/
export KSRC=/lib/modules/`uname -r`/build
export ARCH=x86
export TARGET_BOX=x86
export TARGET_PLATFORM=X86
export LIB_BOARD_DIR=lib-x86
export SYSROOT_USR_LIB_PATH=/usr/lib/i386-linux-gnu
export SYSROOT_EXTN_PATH=$SYSROOT_USR_LIB_PATH
