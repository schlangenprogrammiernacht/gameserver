#!/bin/sh -e

action="$1"

echo "Hello from the SPN C++ container!"

case "$action" in
	compile)
		# copy the code in, build and move the binary out
		cd /spnbot/spn_cpp_framework/

		cp -f /spndata/usercode.cpp src/usercode.cpp
		./build.sh && cp build/bot /spndata/bot
		;;

	run)
		# run the bot and allow coredumps
		ulimit -c unlimited

		cd /spndata/
		exec ./bot
		;;

	*)
		echo "Invalid action: $action"
		exit 1
esac
