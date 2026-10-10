#!/bin/bash
set -e
REPO=$(cd "$(dirname "$0")/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
mkdir -p $S
cd $S
curl -sLO https://cdimage.ubuntu.com/ubuntu-base/releases/24.04/release/ubuntu-base-24.04.5-base-amd64.tar.gz
echo "e77b6f10c2590cef872b33ee9f635a0e3fd1f57fb074c0e52b5c7f56147a0c86  ubuntu-base-24.04.5-base-amd64.tar.gz" | sha256sum -c
rm -rf rootfs; mkdir rootfs; unshare -r tar xpf ubuntu-base-24.04.5-base-amd64.tar.gz -C rootfs 2>/dev/null || true
curl -sL -o rootfs/root/libsodium.tar.gz https://github.com/jedisct1/libsodium/releases/download/1.0.22-RELEASE/libsodium-1.0.22.tar.gz
R() { bwrap --unshare-user --uid 0 --gid 0 --bind $S/rootfs / --dev /dev --proc /proc --tmpfs /tmp --bind $REPO /src --ro-bind /etc/resolv.conf /etc/resolv.conf --setenv HOME /root --setenv PATH /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin --setenv DEBIAN_FRONTEND noninteractive "$@"; }
R bash -c 'apt-get -o APT::Sandbox::User=root update -q >/dev/null 2>&1; apt-get -o APT::Sandbox::User=root install -y -q clang-18 lld-18 libc++-18-dev libc++abi-18-dev cmake ninja-build libssl-dev make git file >/root/apt.log 2>&1; which clang-18 ninja cmake'
R bash -c 'cd /root && echo "adbdd8f16149e81ac6078a03aca6fc03b592b89ef7b5ed83841c086191be3349  libsodium.tar.gz" | sha256sum -c && tar xzf libsodium.tar.gz --no-same-owner && cd libsodium-1.0.22 && CC=clang-18 ./configure --disable-shared -q >/dev/null && make -j8 -s >/dev/null 2>&1 && make install -s >/dev/null 2>&1 && ls /usr/local/lib/libsodium.a'
R bash -c 'git config --global --add safe.directory "*"; B=/src/build/Release-x64-ubuntu2404-headless; mkdir -p $B && cd $B && CC=clang-18 CXX=clang++-18 LDFLAGS="-L /usr/local/lib" cmake -DARCHITECTURE=x64 -DCMAKE_BUILD_TYPE=Release -D_CMAKE_TOOLCHAIN_PREFIX=llvm- -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DHEADLESS=1 -DGENERATE_DEBUG_INFORMATION=0 /src -G Ninja >/root/cm.log 2>&1 && nice -n 19 ninja -j10 Hypersomnia > /root/ninja.log 2>&1; echo ninja $?; grep -E "error:" /root/ninja.log | head; ldd Hypersomnia | awk "{print \$1}" | tr "\n" " "; echo; objdump -T Hypersomnia | grep -oE "GLIBC_[0-9.]+" | sort -Vu | tail -1'
