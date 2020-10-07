# XCEC

A three-valued logic equivalence checking approach based on FSM

## build z3

``` shell
mkdir build & cd build
cmake -DCMAKE_INSTALL_PREFIX=~/XCEC -DCMAKE_INSTALL_INCLUDEDIR=include/z3 -DCMAKE_BUILD_TYPE=Release -DZ3_BUILD_LIBZ3_SHARED=OFF -DZ3_ENABLE_EXAMPLE_TARGETS=OFF -DZ3_BUILD_PYTHON_BINDINGS=OFF -DZ3_BUILD_DOTNET_BINDINGS=OFF -DZ3_BUILD_JAVA_BINDINGS=OFF -DZ3_BUILD_DOCUMENTATION=OFF -DZ3_BUILD_TEST_EXECUTABLES=OFF -DZ3_ENABLE_TRACING_FOR_NON_DEBUG=OFF ..
make install
```

## build cryptominisat

``` shell
mkdir build & cd build
cmake -DENABLE_PYTHON_INTERFACE=OFF -DSTATICCOMPILE=ON -DENABLE_TESTING=OFF -DNOM4RI=ON -DREQUIRE_M4RI=OFF -DONLY_SIMPLE=ON -DCMAKE_INSTALL_PREFIX=../../stp ..
make install
```

## build stp

``` shell
sudo apt-get install bison flex libboost-all-dev minisat
mkdir build & cd build
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_PYTHON_INTERFACE=OFF -DPYTHON_EXECUTABLE=OFF -DENABLE_TESTING=OFF -DSTATICCOMPILE=ON -DCMAKE_INSTALL_PREFIX=../../../../XCEC ..
make install
```

## build boolector

``` shell
cd boolector
./contrib/setup-btor2tools.sh
./contrib/setup-cadical.sh
./contrib/setup-cms.sh
./configure.sh --prefix ../../../../XCEC
cd build && make install
```

## build

``` shell
mkdir build && cd build
cmake ..
make
```

## run

`./XCEC <golden.v> <revised.v> <output> <i/u> <timeout(-1 unlimited) max_conflicts(-1 unlimited)>`

example:

``` shell
cd build
./XCEC ../test/case2/gf.v ../test/case2/rf.v ../output/output_2.txt
```

## reference

<https://zhuanlan.zhihu.com/p/92659252>  
<https://www.csee.usf.edu/~haozheng/teach/cda5416/>  
<https://courses.cs.washington.edu/courses/cse507/17wi/>
