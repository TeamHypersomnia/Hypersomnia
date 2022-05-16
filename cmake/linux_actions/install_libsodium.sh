wget https://github.com/jedisct1/libsodium/releases/download/1.0.16/libsodium-1.0.16.tar.gz -O /tmp/libsodium.tar.gz
pushd .
cd /tmp
tar -zxvf /tmp/libsodium.tar.gz
cd libsodium-*
./configure --disable-shared
make
sudo make install
popd
sudo ldconfig
