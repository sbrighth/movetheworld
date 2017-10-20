#!/bin/bash

PROG_NAME="mount.sh"
PROG_VER="v1.0"

help()
{
	echo "[${PROG_NAME} ${PROG_VER}]"
	echo "ex) ${PROG_NAME} on/off"
}

if [ $# -lt 1 ]
then
	help
	exit -1;
elif [ $1 == "on" ]
then
	mount -t cifs -o rw,username=admin,password=Exicon777 //192.168.100.250/athost /tmp/exicon/athost
elif [ $1 == "off" ]
then
	umount /tmp/exicon/athost
else
	help
fi

exit 0;
