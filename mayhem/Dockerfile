FROM debian:stable-slim as builder

COPY . /faust
WORKDIR /faust

RUN \
  apt-get update && \
  apt-get install -y build-essential llvm libncurses5-dev libncurses5 libmicrohttpd-dev git cmake pkg-config clang

ENV DESTDIR=/install
RUN make -j8 
RUN make libsall -j8
RUN make install
RUN clang++ /faust/mayhem/fuzz_boxInt.cpp -fsanitize=fuzzer,address -I /faust/compiler/boxes/ -I /install/usr/local/include/ -I /faust/compiler/tlib/ -I /faust/compiler/ -I /faust/compiler/errors/ -L /faust/build/lib/ -l:libfaust.a -o /fuzz

FROM debian:stable-slim as package
COPY --from=builder /install /install
COPY --from=builder /fuzz /fuzz_boxInt
