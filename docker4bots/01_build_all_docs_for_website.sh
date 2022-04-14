#!/bin/sh

TARGET_DIR="../../website/docs/static/docs"

for d in spn_*_base
do
	PROGLANG=${d%_base}
	PROGLANG=${PROGLANG#spn_}
	echo ">>> Building $PROGLANG documentation..."
	./1_build_doc.sh "$PROGLANG" "$TARGET_DIR/$PROGLANG"
	echo ">>> Done."
done
