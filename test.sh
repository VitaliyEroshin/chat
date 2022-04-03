./clear.sh
cmake -B build -DTEST=on
cmake --build build -j4

./tests/bin/encoder_test
