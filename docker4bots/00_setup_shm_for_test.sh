#!/bin/sh

set -e

sudo mount -t tmpfs -o noexec,size=1G none /mnt/spn_shm/
sudo chown thomas:users /mnt/spn_shm
mkdir /mnt/spn_shm/testbot

