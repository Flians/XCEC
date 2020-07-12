# XCEC

A three-valued logic equivalence checking approach based on FSM

## build z3

```
sudo apt install musl-tools
cd z3
mkdir build & cd build
cmake -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_BUILD_TYPE=Release -DZ3_BUILD_LIBZ3_SHARED=OFF -DZ3_SINGLE_THREADED=ON ..
make
make install
```

## build

``` shell
mkdir build && cd build
cmake ..
make
```

## run

``` shell
cd build
./XCEC ../test/case2/gf.v ../test/case2/rf.v ../output/output_2.txt
```
