FROM ubuntu:bionic AS builder

RUN apt-get update -qy && \
    apt-get install -y software-properties-common wget && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    add-apt-repository -y "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-7 main" && \
    apt-get update -qy && \
    apt-get upgrade -qy && \
    apt-get install -qy \
      clang-7 \
      cmake \
      git \
      libc++-7-dev \
      libc++abi-7-dev \
      libmbedtls-dev \
      libsodium-dev \
      libssl-dev \
      lld-7 \
      llvm \
      ninja-build \
      openssl

ENV CXX=clang++-7
ENV CC=clang-7
ENV BUILD_FOLDER_SUFFIX=headless

COPY . /hypersomnia
WORKDIR /hypersomnia

RUN cmake/build.sh Release x64 "-DHYPERSOMNIA_DEDICATED_SERVER=1 -DGENERATE_DEBUG_INFORMATION=0" && \
    ninja all -C build/current

FROM ubuntu:bionic

COPY --from=builder /hypersomnia/hypersomnia /hypersomnia
COPY --from=builder /hypersomnia/build/current/Hypersomnia /hypersomnia/Hypersomnia
COPY --from=builder /usr/lib /usr/lib
COPY --from=builder /hypersomnia/hypersomnia/user/config.lua.example /hypersomnia/user/config.lua

WORKDIR /hypersomnia
EXPOSE 8412/udp
CMD ["./Hypersomnia", "--dedicated-server"]
