mkdir build
cd build

BUILD_CONFIG=bootstrap-debug \
../configure \
  --enable-languages=c,c++ \
  --disable-multilib

make -j2
