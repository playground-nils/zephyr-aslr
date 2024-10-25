#!/bin/bash

# This file is meant as an automatic benchmark toggling kconfig parameters
# with a python script for analyzing the results
# How to use :
# execute from the root of the project,
# - ./create_and_flash.sh
# repeat 4 times,
# either,
# - press FUN three times
# or
# - wait to to be prompted to press FUN three times

ZEPHYR_IMG=build/zephyr/zephyr.img
ZEPHYR_BIN=build/zephyr/zephyr.bin
RUNNER_SCRIPT=query_serial.py
LOAD_ADDR=0x10000000
EMMC_LOAD=0x100000
SPL="NOT SET"

function make_image () {
	west build -b khadas_edge2 --pristine
	if [ $? -eq 1 ] ; then
		exit 1
	fi
	mkimage -C none -A arm64 -O linux -a $LOAD_ADDR -e $LOAD_ADDR -d $ZEPHYR_BIN $ZEPHYR_IMG
}

function prepare_board () {
	rkdeveloptool db $SPL
}

function flash_image () {
	rkdeveloptool wl $EMMC_LOAD $ZEPHYR_IMG
	rkdeveloptool rd
}

function swap_config () {
    mv prj.conf temp
    mv other prj.conf
    mv temp other
}

function toggle_param () {
	param=$(cat "prj.conf" | rg $1 | tail -c 2)
	case $param in
		y*)
			sed -i -e "s/$1=y/$1=n/g" "prj.conf";;
		n* )
			sed -i -e "s/$1=n/$1=y/g" "prj.conf";;
	esac
}

function wait_for_maskrom() {
	while : ; do
		rkdeveloptool ld 2>& 1 > /dev/null
		if [ $? -eq 1 ] ; then
			echo "Ensure that the device is in maskrom mode"
			read  -n 1 -p "Press any key to resume" _
		else
			break
		fi
	done
}

function check_spl () {
    if [ "$SPL" = "NOT SET" ] ; then
        # using an env var should be better
        echo "Set the path to the SPL"
        exit 1
    fi
}

check_spl

# Testing 2 bool parameters
for _ in {0..1}; do
	for _ in {0..1}; do
		make_image
		wait_for_maskrom
		prepare_board
		flash_image
		sleep 5s

		python $RUNNER_SCRIPT

        # first param
		toggle_param "CONFIG_BENCHMARKING"
	done
    # second param
	toggle_param "SCTLR_BENCHMARKING"
done

