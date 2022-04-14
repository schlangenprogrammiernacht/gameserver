#!/bin/bash -e

source $(dirname $0)/config.sh

usage() {
	echo "usage: $0 <programming-language> <output-directory>"
}

PROGLANG="$1"
OUTPUT_DIR="$(readlink -f $2)"

if [ -z "$PROGLANG" ]; then
	echo "Argument required: programming language"
	usage
	exit 1
fi

if [ -z "$OUTPUT_DIR" ]; then
	echo "Argument required: output directory"
	usage
	exit 1
fi

mkdir -p "$OUTPUT_DIR"
chmod 777 "$OUTPUT_DIR"

docker run --rm \
	$DOCKER_COMPILE_ARGS \
	-v "$OUTPUT_DIR:/doc:rw" \
	--name "doc_${BOT_NAME}_${VERSION_ID}" \
	spn_${PROGLANG}_base:latest doc

chmod 755 "$OUTPUT_DIR"
