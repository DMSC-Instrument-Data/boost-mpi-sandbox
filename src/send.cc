#include <stdio.h>
#include <algorithm>
#include <chrono>

#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>

namespace mpi = boost::mpi;

// BOOST_IS_MPI_DATATYPE(std::vector<double>) // I think this would give nonsense
// BOOST_CLASS_TRACKING(std::vector<double>,track_never) // does not help
// BOOST_CLASS_IMPLEMENTATION(std::vector<double>,object_serializable) // does not help

void send(mpi::communicator &comm, std::vector<double> &data) {
  auto size = static_cast<int>(data.size());
  int tag = 0;
  if (comm.rank() == 0) {
    // fast (except for short length vectors)
    comm.send(1, tag, size);
    comm.send(1, tag, data.data(), size);

    // fast (except for short/medium length vectors)
    // comm.send(1, tag, mpi::skeleton(data));
    // comm.send(1, tag, mpi::get_content(data));

    // fast
    // comm.send(1, tag, data.data(), size);

    // 2 orders of magnitude slower
    // comm.send(1, tag, data);
  } else {
    int length;
    comm.recv(0, tag, length);
    data.resize(length);
    comm.recv(0, tag, data.data(), length);

    // comm.recv(0, tag, mpi::skeleton(data));
    // comm.recv(0, tag, mpi::get_content(data));

    // comm.recv(0, tag, data.data(), size);

    // comm.recv(0, tag, data);
  }
}

template <class... Args>
double run(const int count, mpi::communicator &comm, Args &&... args) {
  comm.barrier();
  const auto start = std::chrono::system_clock::now();
  for (int r = 0; r < count; ++r) {
    send(comm, args...);
  }
  const auto end = std::chrono::system_clock::now();
  comm.barrier();
  std::chrono::duration<double> elapsed = end - start;
  return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed)
             .count() /
         (1000000000);
}

template <class... Args>
std::pair<double, int> benchmark(mpi::communicator &comm, Args &&... args) {
  // Trial run to compute repeat for running roughly one second
  const auto trial = run(100, comm, args...);
  int repeat = std::max(100, static_cast<int>(1.0 / (trial / 100)));
  mpi::broadcast(comm, repeat, 0);
  return {run(repeat, comm, args...), repeat};
}

int main(int argc, char **argv) {
  mpi::environment env;
  mpi::communicator comm;

  auto size = std::max(1, atoi(argv[1]) / static_cast<int>(sizeof(double)));
  std::vector<double> data(size);

  double seconds;
  int repeat;
  std::tie(seconds, repeat) = benchmark(comm, data);

  double MB = static_cast<double>(size * sizeof(double)) / (1024 * 1024);
  if (comm.rank() == 0)
    printf("%lf MB %lf seconds bandwidth %lf MB/s\n", MB, seconds,
           repeat * MB / seconds);

  return 0;
}
