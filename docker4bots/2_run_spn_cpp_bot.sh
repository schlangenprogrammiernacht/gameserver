#!/bin/sh

if [ -z "$1" ]; then
	echo "Argument required: bot name"
	exit 1
fi

docker run \
	--memory=32M --memory-swap=32M --cpus=1 \
	--tmpfs /run --tmpfs /tmp \
	spn_cpp_bot:$1
