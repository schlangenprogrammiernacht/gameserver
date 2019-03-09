FROM spn_cpp_base:latest

COPY usercode.cpp /spnbot/spn_cpp_framework/src/usercode.cpp
RUN cd /spnbot/spn_cpp_framework/src && chown spnbot * && chmod 644 *

USER spnbot

RUN cd /spnbot/spn_cpp_framework && ./build.sh

# the wrapper script (see base image) is run as root to set up the container at
# runtime. That script runs the bot code as user spnbot.
USER root
