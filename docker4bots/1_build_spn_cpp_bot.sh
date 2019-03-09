#!/bin/sh

if [ -z "$1" ]; then
	echo "Argument required: snake version id"
	exit 1
fi

if [ -z "$2" ]; then
	echo "Argument required: path to user code file"
	exit 1
fi

VERSION_ID=$1
CODE_FILE=$2

CONTEXTDIR=$(mktemp -d)

cp spn_cpp_bot/Dockerfile "$CONTEXTDIR"
cp "$CODE_FILE" "$CONTEXTDIR/usercode.cpp"

pushd "$CONTEXTDIR"

docker build -t spn_cpp_bot:$VERSION_ID .
EXITCODE=$?

popd

rm -r "$CONTEXTDIR"

exit $EXITCODE
