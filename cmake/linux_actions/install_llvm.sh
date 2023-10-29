wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo add-apt-repository -y "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main"
sudo apt-get -q update
sudo apt-get -y install clang-16 lld-16 libc++-16-dev libc++abi-16-dev
