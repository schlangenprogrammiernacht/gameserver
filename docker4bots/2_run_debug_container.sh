#!/bin/sh -x

SPN_SHM_HOSTDIR="/mnt/spn_shm"

BOTNAME="$1"
CONTAINERNAME="$2"

if [ -z "$BOTNAME" ]; then
	echo "Argument required: bot name"
	exit 1
fi

docker run \
	--memory=32M --memory-swap=32M --cpus=1 \
	--read-only --tmpfs /run --tmpfs /tmp \
	-v $SPN_SHM_HOSTDIR/$BOTNAME:/spnshm \
	--network none \
	--name "$CONTAINERNAME" \
	--entrypoint=/bin/bash -it \
	spn_cpp_bot:$BOTNAME
