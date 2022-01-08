#!/bin/bash

source $(dirname $0)/config.sh

usage() {
	echo "usage: $0 <programming-language> <version-id> <bot-name> <container-name>"
}

PROGLANG="$1"
VERSION_ID="$2"
BOT_NAME="$3"
CONTAINER_NAME="$4"

if [ -z "$PROGLANG" ]; then
	echo "Argument required: programming language"
	usage
	exit 1
fi

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
	-v $SPN_SHM_HOSTDIR/$BOT_NAME:/spnshm \
	--name "$CONTAINER_NAME" \
	"spn_${PROGLANG}_base:latest" run
