# XCEC

A three-valued logic equivalence checking approach based on FSM

## build z3

```
mkdir build & cd build
cmake -DCMAKE_INSTALL_PREFIX=~/XCEC -DCMAKE_INSTALL_INCLUDEDIR=include/z3 -DCMAKE_BUILD_TYPE=Release -DZ3_BUILD_LIBZ3_SHARED=ON -DZ3_SINGLE_THREADED=ON ..
make install
```

## build cryptominisat

```
mkdir build & cd build
cmake -DSTATICCOMPILE=ON -DREQUIRE_M4RI=OFF -DCMAKE_INSTALL_PREFIX=../../stp ..
make install
```

## build stp

```
sudo apt-get install bison flex libboost-all-dev minisat
mkdir build & cd build
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_PYTHON_INTERFACE=OFF  -DSTATICCOMPILE=ON -DCMAKE_INSTALL_PREFIX=../../XCEC ..
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

## reference

https://zhuanlan.zhihu.com/p/92659252
https://www.csee.usf.edu/~haozheng/teach/cda5416/
https://courses.cs.washington.edu/courses/cse507/17wi/