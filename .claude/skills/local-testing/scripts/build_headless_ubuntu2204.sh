#!/bin/bash
# Headless Release build inside an unprivileged Ubuntu 22.04 rootfs (bwrap): glibc 2.35, so the binary runs on
# both arenaus (22.04) and arena (25.04). clang-18 and libc++ from apt.llvm.org (key fingerprint checked),
# libsodium 1.0.22 from the release tarball (checksum pinned). Output: build/Release-x64-ubuntu22-headless/Hypersomnia
set -e
REPO=$(cd "$(dirname "$0")/../../../.." && pwd)
S=${SCRATCH:-/tmp/hypersomnia-nettest}
mkdir -p $S
# Outside /tmp by default: a reboot clears /tmp, and the rootfs takes minutes to rebuild
R=${ROOTFS22:-$HOME/.cache/hypersomnia-rootfs22}

run() {
	bwrap --unshare-user --uid 0 --gid 0 --bind $R / --dev /dev --proc /proc --tmpfs /tmp --bind $REPO /src \
		--ro-bind /etc/resolv.conf /etc/resolv.conf --setenv HOME /root \
		--setenv PATH /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin --setenv DEBIAN_FRONTEND noninteractive "$@"
}

if [ ! -x $R/usr/bin/clang-18 ] || [ ! -f $R/usr/local/lib/libsodium.a ]; then
	cd $S
	curl -sLO https://cdimage.ubuntu.com/ubuntu-base/releases/22.04/release/ubuntu-base-22.04.5-base-amd64.tar.gz
	echo "242cd8898b33ea806ef5f13b1076ed7c76f9f989d18384452f7166692438ff1a  ubuntu-base-22.04.5-base-amd64.tar.gz" | sha256sum -c
	rm -rf $R; mkdir -p $R
	unshare -r tar xpf ubuntu-base-22.04.5-base-amd64.tar.gz -C $R 2>/dev/null || true

	run bash -c 'apt-get -o APT::Sandbox::User=root update -q >/dev/null 2>&1; apt-get -o APT::Sandbox::User=root install -y -q wget gnupg ca-certificates >/dev/null 2>&1 || true'
	run bash -c 'wget -qO /tmp/llvm.asc https://apt.llvm.org/llvm-snapshot.gpg.key && gpg --show-keys --with-fingerprint /tmp/llvm.asc 2>/dev/null | tr -d " " | grep -q 6084F3CF814B57C1CF12EFD515CF4D18AF4F7421 && gpg --dearmor < /tmp/llvm.asc > /etc/apt/trusted.gpg.d/llvm.gpg && echo "deb https://apt.llvm.org/jammy/ llvm-toolchain-jammy-18 main" > /etc/apt/sources.list.d/llvm.list'
	run bash -c 'apt-get -o APT::Sandbox::User=root update -q >/dev/null 2>&1; apt-get -o APT::Sandbox::User=root install -y -q clang-18 lld-18 libc++-18-dev libc++abi-18-dev cmake ninja-build libssl-dev make git file >/root/apt.log 2>&1 || true; which clang-18 ninja cmake >/dev/null'

	curl -sL -o $R/root/libsodium.tar.gz https://github.com/jedisct1/libsodium/releases/download/1.0.22-RELEASE/libsodium-1.0.22.tar.gz
	run bash -c 'cd /root && echo "adbdd8f16149e81ac6078a03aca6fc03b592b89ef7b5ed83841c086191be3349  libsodium.tar.gz" | sha256sum -c && tar xzf libsodium.tar.gz --no-same-owner && cd libsodium-1.0.22 && CC=clang-18 ./configure --disable-shared -q >/dev/null && make -j8 -s >/dev/null 2>&1 && make install -s >/dev/null 2>&1'
fi

run bash -c 'git config --global --add safe.directory "*"; B=/src/build/Release-x64-ubuntu22-headless; mkdir -p $B && cd $B && CC=clang-18 CXX=clang++-18 LDFLAGS="-L /usr/local/lib" cmake -DARCHITECTURE=x64 -DCMAKE_BUILD_TYPE=Release -D_CMAKE_TOOLCHAIN_PREFIX=llvm- -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DHEADLESS=1 -DGENERATE_DEBUG_INFORMATION=0 /src -G Ninja >/root/cm.log 2>&1 && nice -n 19 ninja -j10 Hypersomnia > /root/ninja.log 2>&1; echo "ninja exit $?"; grep -E "error:" /root/ninja.log | head; echo "max glibc: $(objdump -T Hypersomnia | grep -oE "GLIBC_[0-9.]+" | sort -Vu | tail -1)"'

mkdir -p $S/libcxx22
for l in libc++.so.1 libc++abi.so.1 libunwind.so.1; do cp -L $R/usr/lib/llvm-18/lib/$l $S/libcxx22/ 2>/dev/null || cp -L $R/lib/x86_64-linux-gnu/$l $S/libcxx22/; done
echo "libc++ to ship: $S/libcxx22"
