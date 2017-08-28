# Boost-MPI-Sandbox
Sandbox for playing with boost::mpi and performance benchmarking in preparation for distributed loading of Nexus files.

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

## TODO

For the application of loading neutron event data from Nexus files we need to send vectors of random lengths (random number of neutrons hitting each pixel) from MPI ranks loading the file to all other MPI ranks that hold the respective pixels of the instrument.
For benchmarking purposes a vector of any type such as `double` is sufficient, whereas actually we will be dealing with spectrum indices, time-of-flight, and pulse time.

1. Try and benchmark non-blocking sends (`boost::mpi::communicator::isend`/`MPI_Isend`) and/or receives (`boost::mpi::communicator::irecv`/`MPI_Irecv`).
2. Try and benchmark bidirectional communication, also generalizing to the case of more than 2 MPI ranks. This is what we will actually need: File is read on multiple/all ranks and then redistributed to the correct rank that holds the data for the corresponding spectra in the workspace.
3. Try and benchmark collective communication (`boost::mpi::all_to_all`/`MPI_Alltoall`) for distributing the vector lengths (see option 3. in send.cc, which currently looks like the best candidate pattern for implementing what we need).
4. Generalize item 2. to the case of M data providers and N data sinks, i.e., the potential case of reading the file on a different number or different subset of ranks than those used for processing.
