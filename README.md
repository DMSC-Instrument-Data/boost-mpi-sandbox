# Boost-MPI-Sandbox
Sandbox for playing with boost::mpi and performance benchmarking

## Building

```
mkdir build
cd build
cmake ..
make
```

## Running

```
mpirun -n 2 ./src/send
```

Test size and different variants can be adapted in the source code (`src/send.cc`).
