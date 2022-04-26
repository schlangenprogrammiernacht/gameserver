#!/bin/sh

PROGLANG="$1"

shift

if [ "x$PROGLANG" = "x" ]; then
	echo "Programming language not set!"
	echo
	echo "usage: $0 <programming-language> [<docker-flags...>]"
	exit 1
fi

BASE_DIR="spn_${PROGLANG}_base"

if [ ! -d "$BASE_DIR" ]; then
	echo "'$BASE_DIR' does not exist or is not a directory!"
	exit 1
fi

docker build -t "spn_${PROGLANG}_base:latest" $@ "$BASE_DIR"
