#!/bin/sh -e

action="$1"

echo "Hello from the SPN Rust container!"

case "$action" in
	compile)
		# copy the code in, build and move the binary out
		cd /spnbot/spn_rust_framework/

		cp -f /spndata/usercode.rs src/usercode.rs
		./build.sh && cp target/release/spn_rust_bot /spndata/bot
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
