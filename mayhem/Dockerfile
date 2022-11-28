FROM debian:stable-slim as builder

COPY . /faust
WORKDIR /faust

RUN \
  apt-get update && \
  apt-get install -y build-essential llvm libncurses5-dev libncurses5 libmicrohttpd-dev git cmake pkg-config && \
  rm -rf /var/lib/apt/lists/*

ENV DESTDIR=/install
RUN \
  make -j8 && make install && \
  make clean && \
  apt-get purge -y build-essential llvm libncurses5-dev && apt-get autoremove -y

FROM debian:stable-slim as package
COPY --from=builder /install /install
