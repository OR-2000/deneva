#!/usr/bin/env bash
# setup.sh (placed inside deneva/)
# Usage:
#   bash setup.sh [username]
# Example:
#   bash setup.sh            # default = hoge
#   bash setup.sh myuser     # custom user

set -euo pipefail

USERNAME="${1:-hoge}"

USER_HOME="/home/${USERNAME}"
DENEVA_DIR="$(pwd)"   # assume this script is run from deneva/
JEMALLOC_PREFIX="${DENEVA_DIR}/jemalloc-4.0.3"
NANOMSG_PREFIX="${DENEVA_DIR}/nanomsg-0.5-beta"

# ------- helper -------
msg(){ printf "\n=== %s ===\n" "$*"; }

# ------- apt -------
msg "Installing system packages (sudo required)"
sudo apt-get update
sudo apt-get install -y \
  build-essential pkg-config git libtool autoconf automake \
  libboost-all-dev

# ------- repos (SSH only) -------
JEMALLOC_URL="git@github.com:jemalloc/jemalloc.git"
NANOMSG_URL="git@github.com:nanomsg/nanomsg.git"

# ------- jemalloc 4.0.3 -------
msg "Building jemalloc 4.0.3 into ${JEMALLOC_PREFIX}"
rm -rf jemalloc
git clone "${JEMALLOC_URL}" jemalloc
cd jemalloc
git checkout 4.0.3
autoconf
./configure --prefix="${JEMALLOC_PREFIX}" --with-version=4.0.3 --with-jemalloc-prefix=je_
make -j
sudo make install
cd "${DENEVA_DIR}"

# ------- nanomsg 0.5-beta -------
msg "Building nanomsg 0.5-beta into ${NANOMSG_PREFIX}"
rm -rf nanomsg
git clone "${NANOMSG_URL}" nanomsg
cd nanomsg
git checkout 0.5-beta
./autogen.sh
./configure --prefix="${NANOMSG_PREFIX}"
make -j
sudo make install
cd "${DENEVA_DIR}"

# ------- build deneva -------
msg "Building deneva"
mkdir -p obj
make deps
make -j

msg "Done."
echo
echo "If needed, add these to your shell profile:"
echo "  export PATH=\"${JEMALLOC_PREFIX}/bin:${NANOMSG_PREFIX}/bin:\$PATH\""
echo "  export LD_LIBRARY_PATH=\"${JEMALLOC_PREFIX}/lib:${NANOMSG_PREFIX}/lib:\$LD_LIBRARY_PATH\""