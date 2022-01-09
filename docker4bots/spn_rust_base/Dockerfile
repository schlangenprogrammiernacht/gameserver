FROM rust:1.57-slim

#RUN apk add --no-cache rust cargo

RUN addgroup --system spnbot && adduser --system --uid 1337 --home /spnbot --shell /bin/sh --ingroup spnbot spnbot

ADD bot_wrapper.sh /

ADD spn_rust_framework/ /spnbot/spn_rust_framework

# pre-build framework, hand it over to the spnbot user and create the shared memory directory
# (all in one step to save layers)
RUN cd /spnbot/spn_rust_framework && ./build.sh && chown -R spnbot /spnbot && mkdir /spnshm
VOLUME /spnshm
VOLUME /spndata

USER spnbot

ENTRYPOINT ["/bot_wrapper.sh"]
