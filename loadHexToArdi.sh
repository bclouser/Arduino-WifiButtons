#!/usr/bin/env bash

DEVICE=atmega328p
PROGRAMMER="avrispmkII -P usb"
HEX_FILE=$1
COPY_HEX_NAME="latest.hex"

if [ -z ${HEX_FILE} ];then
	echo "Pass in the hexfile as argument 1!!!"
	exit 1
fi

#This is just for fun.
cp ${HEX_FILE} ./${COPY_HEX_NAME}

echo "avrdude -c ${PROGRAMMER} -p ${DEVICE} -U flash:w:${HEX_FILE}:i"
avrdude -c ${PROGRAMMER} -p ${DEVICE} -U flash:w:${HEX_FILE}:i
