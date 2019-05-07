#!/bin/bash

source config.sh

usage() {
	echo "usage: $0 <version-id> <bot-name> <code-file>"
}

VERSION_ID=$1
BOT_NAME=$2
CODE_FILE=$3

if [ -z "$VERSION_ID" ]; then
	echo "Argument required: snake version id"
	usage
	exit 1
fi

if [ -z "$BOT_NAME" ]; then
	echo "Argument required: bot name"
	usage
	exit 1
fi

if [ -z "$CODE_FILE" ]; then
	echo "Argument required: path to user code file"
	usage
	exit 1
fi

BOT_DATADIR="$SPN_DATA_HOSTDIR/${BOT_NAME}_$VERSION_ID"

mkdir -p "$BOT_DATADIR"
chmod 777 "$BOT_DATADIR"

install -m 444 "$CODE_FILE" "$BOT_DATADIR/usercode.cpp"

exec docker run --rm \
	$DOCKER_COMPILE_ARGS \
	-v "$BOT_DATADIR:/spndata:rw" \
	--name "build_${BOT_NAME}_${VERSION_ID}" \
	spn_cpp_base:latest compile
