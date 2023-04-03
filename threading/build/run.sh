cmake -DCMAKE_BUILD_TYPE=Release -GNinja ..
ninja -j 16
./test1 --benchmark-repetitions=20