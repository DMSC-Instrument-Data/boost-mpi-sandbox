#include <boost/mpi.hpp>
#include <iostream>
#include <chrono>
#include <boost/serialization/vector.hpp>
namespace mpi = boost::mpi;

//BOOST_IS_MPI_DATATYPE(std::vector<double>) // I think this would give nonesense
//BOOST_CLASS_TRACKING(std::vector<double>,track_never) // does not help
//BOOST_CLASS_IMPLEMENTATION(std::vector<double>,object_serializable) // does not help

int main(int argc, char **argv)
{
  mpi::environment env;
  mpi::communicator comm;

  auto repeat = atoi(argv[1]);
  auto size = atoi(argv[2]);
  std::vector<double> data(size);
  int tag = 0;

  const auto start = std::chrono::system_clock::now();

  if (comm.rank() == 0) {
    for (int r = 0; r < repeat; ++r) {
      // fast (except for short length vectors)
      comm.send(1, tag, size);
      comm.send(1, tag, data.data(), size);

      // fast (except for short/medium length vectors)
      //comm.send(1, tag, mpi::skeleton(data));
      //comm.send(1, tag, mpi::get_content(data));

      // fast
      //comm.send(1, tag, data.data(), size);
      
      // 2 orders of magnitude slower
      //comm.send(1, tag, data);
    }
  } else {
    for (int r = 0; r < repeat; ++r) {
      int length;
      comm.recv(0, tag, length);
      data.resize(length);
      comm.recv(0, tag, data.data(), length);

      //comm.recv(0, tag, mpi::skeleton(data));
      //comm.recv(0, tag, mpi::get_content(data));

      //comm.recv(0, tag, data.data(), size);

      //comm.recv(0, tag, data);
    }
  }

  const auto end = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed = end - start;
  double seconds = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                       elapsed).count() /
                   (1000000000);

  printf("%f bandwidth %f MB/s\n", seconds,
         static_cast<double>(repeat * size * sizeof(double)) / seconds /
             (1024 * 1024));

  return 0;
}
