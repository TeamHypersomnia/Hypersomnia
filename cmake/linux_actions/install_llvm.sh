wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo add-apt-repository -y "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-20 main"
sudo apt-get -q update
sudo apt-get -y install clang-20 lld-20 libc++-20-dev libc++abi-20-dev
