#!/bin/bash

source $(dirname $0)/config.sh

usage() {
	echo "usage: $0 <version-id> <bot-name> <container-name>"
}

VERSION_ID="$1"
BOT_NAME="$2"
CONTAINER_NAME="$3"

if [ -z "$VERSION_ID" ]; then
	echo "Argument required: version id"
	usage
	exit 1
fi

if [ -z "$BOT_NAME" ]; then
	echo "Argument required: bot name"
	usage
	exit 1
fi

if [ -z "$CONTAINER_NAME" ]; then
	echo "Argument required: container name"
	usage
	exit 1
fi

BOT_DATADIR="$SPN_DATA_HOSTDIR/${BOT_NAME}_$VERSION_ID"

exec docker run -d --rm \
	$DOCKER_RUN_ARGS \
	-v "$BOT_DATADIR:/spndata:ro" \
	-v "$SPN_SHM_HOSTDIR/$BOT_NAME:/spnshm" \
	--name "$CONTAINER_NAME" \
	spn_cpp_base:latest run
