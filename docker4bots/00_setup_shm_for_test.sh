#!/bin/bash -e

if [ '$EUID' -ne 0 ]
    then echo 'This script needs to be run as root.'
    exit
fi

source config.sh

mkdir -p $SPN_SHM_HOSTDIR/
mount -t tmpfs -o noexec,size=1G none $SPN_SHM_HOSTDIR/
chown $(id -u):$(id -g) $SPN_SHM_HOSTDIR
mkdir $SPN_SHM_HOSTDIR/testbot
