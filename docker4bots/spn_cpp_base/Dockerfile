FROM alpine

RUN apk add --no-chache g++ cmake strace make

RUN addgroup -S spnbot && adduser --uid 1337 --home /spnbot -S spnbot --shell /bin/sh -G spnbot

ADD bot_wrapper.sh /

ADD spn_cpp_framework/ /spnbot/spn_cpp_framework
RUN chown -R spnbot /spnbot

RUN mkdir /spnshm
VOLUME /spnshm

ENTRYPOINT ["/bot_wrapper.sh"]
