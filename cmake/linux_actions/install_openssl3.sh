#!/bin/bash

# Exit script on any error
set -e

# Update local package index
sudo apt update

# Install dependencies for building OpenSSL
sudo apt install -y build-essential checkinstall zlib1g-dev

# Define OpenSSL version
OPENSSL_VERSION="openssl-3.0.8"

# Download OpenSSL from the official source
sudo wget "https://github.com/openssl/openssl/releases/download/${OPENSSL_VERSION}/${OPENSSL_VERSION}.tar.gz"

# Extract the downloaded tarball
sudo tar xvf "${OPENSSL_VERSION}.tar.gz"

# Navigate to the extracted OpenSSL directory
cd "${OPENSSL_VERSION}/"

# Configure OpenSSL
./config

# Build OpenSSL
make

# Run tests (optional, can be skipped with 'make -k test' if some tests fail)
make test

# Install OpenSSL
sudo make install

# Update links and cache for shared libraries
sudo ldconfig

# Update system-wide OpenSSL configuration
sudo tee /etc/profile.d/openssl.sh <<EOF
export PATH=/usr/local/bin:\$PATH
export LD_LIBRARY_PATH=/usr/local/lib:\$LD_LIBRARY_PATH
EOF

# Reload shell environment
source /etc/profile.d/openssl.sh

# Verify OpenSSL installation
openssl version

# Output the installed OpenSSL version
echo "OpenSSL installation completed. Version: $(openssl version)"
