#!/bin/sh

for d in spn_*_base
do
	PROGLANG=${d%_base}
	PROGLANG=${PROGLANG#spn_}
	echo ">>> Building $PROGLANG container..."
	./0_build_spn_base_container.sh "$PROGLANG"
	echo ">>> Done."
done
