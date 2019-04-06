#!/bin/sh

SPN_SHM_HOSTDIR="/mnt/spn_shm"

IMAGENAME="$1"
BOTNAME="$2"
CONTAINERNAME="$3"

if [ -z "$BOTNAME" ]; then
	echo "Argument required: bot name"
	exit 1
fi

exec docker run --rm \
	--memory=32M --memory-swap=32M --cpus=1 \
	--read-only --tmpfs /run --tmpfs /tmp \
	-v $SPN_SHM_HOSTDIR/$BOTNAME:/spnshm \
	--network none \
	--name "$CONTAINERNAME" \
	spn_cpp_bot:$IMAGENAME
