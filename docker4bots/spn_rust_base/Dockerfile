FROM rust:1.66-slim

# Create the shared memory directory and add spnbot user
RUN mkdir /spnshm && \
    addgroup --system spnbot && adduser --system --uid 1337 --home /spnbot --shell /bin/sh --ingroup spnbot spnbot
USER spnbot

COPY --chown=spnbot:spnbot bot_wrapper.sh /
COPY --chown=spnbot:spnbot spn_rust_framework/ /spnbot/spn_rust_framework

# Pre-build framework to reduce compile times
RUN cd /spnbot/spn_rust_framework && \
    ./build.sh

VOLUME /spnshm
VOLUME /spndata

ENTRYPOINT ["/bot_wrapper.sh"]
