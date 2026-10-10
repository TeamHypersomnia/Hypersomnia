wget https://github.com/jedisct1/libsodium/releases/download/1.0.22-RELEASE/libsodium-1.0.22.tar.gz -O /tmp/libsodium.tar.gz
echo "adbdd8f16149e81ac6078a03aca6fc03b592b89ef7b5ed83841c086191be3349  /tmp/libsodium.tar.gz" | sha256sum -c - || exit 1
pushd .
cd /tmp
tar -zxvf /tmp/libsodium.tar.gz
cd libsodium-*
./configure --disable-shared
make
sudo make install
popd
sudo ldconfig
