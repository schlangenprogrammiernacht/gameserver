#!/bin/sh

docker container prune -f
rm -r /mnt/spn_shm/*/
