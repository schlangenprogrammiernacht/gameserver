#!/bin/sh

if [ -z "$1" ]; then
	echo "Argument required: bot name"
	exit 1
fi

docker build -t spn_cpp_bot:$1 spn_cpp_bot/
