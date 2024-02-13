FROM ubuntu:jammy AS builder

RUN apt-get update -qy && \
    apt-get install -y software-properties-common wget && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    add-apt-repository -y "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main" && \
    apt-get update -qy && \
    apt-get upgrade -qy && \
    apt-get install -qy \
      clang-16 \
      cmake \
      git \
      libc++-16-dev \
      libc++abi-16-dev \
      libmbedtls-dev \
      libsodium-dev \
      libssl-dev \
      lld-16 \
      llvm \
      ninja-build \
      openssl

ENV CXX=clang++-16
ENV CC=clang-16
ENV BUILD_FOLDER_SUFFIX=headless

COPY . /hypersomnia
WORKDIR /hypersomnia

RUN cmake/build.sh Release x64 "-DHEADLESS=1" && \
    ninja all -C build/current

FROM ubuntu:jammy

RUN apt-get update -qy && \
    apt-get install -y software-properties-common wget && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    add-apt-repository -y "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main" && \
    apt-get update -qy && \
    apt-get upgrade -qy && \
    apt-get install -qy \
      libc++abi1-16 \
      libc++1-16 \
      libsodium23 \
      libmbedcrypto7 \
      libmbedtls14 \
      libmbedx509-1 \
      openssl

RUN useradd -m hypersomnia
COPY --from=builder /hypersomnia/hypersomnia /home/hypersomnia/hypersomnia
COPY --from=builder /hypersomnia/build/current/Hypersomnia /home/hypersomnia/hypersomnia/Hypersomnia
#COPY --from=builder /hypersomnia/hypersomnia/user/config.lua.example /hypersomnia/user/config.lua
RUN chown -R hypersomnia /home/hypersomnia/hypersomnia
RUN chgrp -R hypersomnia /home/hypersomnia/hypersomnia
USER hypersomnia

WORKDIR /home/hypersomnia/hypersomnia
EXPOSE 8412/udp
CMD ["./Hypersomnia", "--dedicated-server"]
