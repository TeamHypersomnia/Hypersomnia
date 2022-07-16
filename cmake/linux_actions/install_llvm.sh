wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo add-apt-repository -y "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-14 main"
sudo apt-get -q update
sudo apt-get -y install clang-14 lld-14 libc++-14-dev libc++abi-14-dev
