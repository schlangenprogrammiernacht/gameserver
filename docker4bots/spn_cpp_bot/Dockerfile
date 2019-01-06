FROM spn_cpp_base:latest

USER spnbot

# TODO: copy user code into the container

RUN cd /spnbot/spn_cpp_framework && ./build.sh

# the wrapper script (see base image) is run as root to set up the container at
# runtime. That script runs the bot code as user spnbot.
USER root
