#!/bin/bash

set -ex

compilter_version="$1"

# Set up env
if [ "${compiler_version}" == "dev" ]; then
    export CLANG_VERSION_SUFFIX="";
else
    export CLANG_VERSION_SUFFIX="-${compiler_version}";
fi

echo "export CC=\"/usr/bin/clang${CLANG_VERSION_SUFFIX}\"" >> /etc/profile.d/clang.sh
echo "export CXX=\"/usr/bin/clang++${CLANG_VERSION_SUFFIX}\"" >> /etc/profile.d/clang.sh

# Install tools needed to add the clang repository
apt-get install --yes --no-install-recommends ca-certificates wget gnupg

wget https://apt.llvm.org/llvm-snapshot.gpg.key -O /etc/apt/keyrings/llvm.gpg

# Add clang repository
echo "deb [signed-by=/etc/apt/keyrings/llvm.gpg] http://apt.llvm.org/trixie/ llvm-toolchain-trixie${CLANG_VERSION_SUFFIX} main" > /etc/apt/sources.list.d/llvm.list
echo "deb-src [signed-by=/etc/apt/keyrings/llvm.gpg] http://apt.llvm.org/trixie/ llvm-toolchain-trixie${CLANG_VERSION_SUFFIX} main" >> /etc/apt/sources.list.d/llvm.list


# Install clang
apt-get update
apt-get install --yes --install-recommends clang${CLANG_VERSION_SUFFIX}
clang_major_version=$(clang${CLANG_VERSION_SUFFIX} --version | grep -Eo "[0-9]+.[0-9]+.[0-9]+" | cut -d'.' -f1)

if [ ${clang_major_version} -gt 11 ]; then
    apt-get install --yes --no-install-recommends libclang-rt-${clang_major_version}-dev;
fi



