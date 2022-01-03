FROM rust:alpine

#RUN apk add --no-cache rust cargo

RUN addgroup -S spnbot && adduser --uid 1337 --home /spnbot -S spnbot --shell /bin/sh -G spnbot

ADD bot_wrapper.sh /

ADD spn_rust_framework/ /spnbot/spn_rust_framework

# pre-build framework, hand it over to the spnbot user and create the shared memory directory
# (all in one step to save layers)
RUN cd /spnbot/spn_rust_framework && ./build.sh && chown -R spnbot /spnbot && mkdir /spnshm
VOLUME /spnshm
VOLUME /spndata

USER spnbot

ENTRYPOINT ["/bot_wrapper.sh"]
