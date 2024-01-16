#!/bin/sh -e

action="$1"

echo "Hello from the SPN Python container!"

case "$action" in
	compile)
		# copy the code in, build and move the binary out
		cd /spnbot/spn_python_framework/

		echo "starting pylint"
		pylint /spndata/usercode.py || pylint-exit $?
		;;

	run)
		# run the bot and allow coredumps
		ulimit -c unlimited
		cd /spnbot/spn_python_framework/src
		./main.py
		;;

	doc)
		cd /spnbot/spn_python_framework/
		doxygen Doxyfile
		mv doc/html/* /doc
		;;

	*)
		echo "Invalid action: $action"
		exit 1
esac
