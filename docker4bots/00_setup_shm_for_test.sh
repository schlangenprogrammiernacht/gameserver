#!/bin/bash -e

source $(dirname $0)/config.sh

sudo mount -t tmpfs -o noexec,size=1G none $SPN_SHM_HOSTDIR/
sudo chown $(id -u):$(id -g) $SPN_SHM_HOSTDIR
mkdir $SPN_SHM_HOSTDIR/testbot

