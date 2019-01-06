FROM ubuntu:18.04

RUN apt update && apt install -y g++ cmake

RUN useradd -d /spnbot -m spnbot

ADD bot_wrapper.sh /

ADD spn_cpp_framework/ /spnbot/spn_cpp_framework

ENTRYPOINT ["/bot_wrapper.sh"]
