#!/bin/sh

echo "Hello World!"

# run spnbot as user spnbot
cd /spnbot/spn_cpp_framework/build/
su -c ./bot spnbot

echo "Goodbye World!"
