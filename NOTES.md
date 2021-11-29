Abseil:
cmake -DCMAKE_CXX_STANDARD=17 -DCMAKE_POSITION_INDEPENDENT_CODE=ON ..
make install

Google Test:
cmake ..
make install

S2:
brew install gflags glog openssl
cmake -DOPENSSL_ROOT_DIR=`brew --prefix openssl` ..
