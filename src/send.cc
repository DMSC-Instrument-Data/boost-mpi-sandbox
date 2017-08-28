#include <stdio.h>
#include <algorithm>
#include <chrono>

#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>

namespace mpi = boost::mpi;

// See http://www.boost.org/doc/libs/1_58_0/doc/html/mpi/tutorial.html#mpi.performance_optimizations.
// I tried to play with this but did not observe any improvement.
// BOOST_IS_MPI_DATATYPE(std::vector<double>) // I think this would give nonsense
// BOOST_CLASS_TRACKING(std::vector<double>,track_never) // does not help
// BOOST_CLASS_IMPLEMENTATION(std::vector<double>,object_serializable) // does not help

// Send block of memory of known size. This is basically what MPI_Send does,
// boost is probalby ismply forwarding internally.
void send_raw_known_size(mpi::communicator &comm, std::vector<double> &data) {
  auto size = static_cast<int>(data.size());
  int tag = 0;
  if (comm.rank() == 0) {
    comm.send(1, tag, data.data(), size);
  } else {
    int length;
    comm.recv(0, tag, data.data(), size);
  }
}

// Send a vector. Boost has to deal with sending the vector length etc.
void send_boost(mpi::communicator &comm, std::vector<double> &data) {
  auto size = static_cast<int>(data.size());
  int tag = 0;
  if (comm.rank() == 0) {
    comm.send(1, tag, data);
  } else {
    comm.recv(0, tag, data);
  }
}

// Send block of memory of unknown size, manually exchange size in first step.
void send_raw_unknown_size(mpi::communicator &comm, std::vector<double> &data) {
  auto size = static_cast<int>(data.size());
  int tag = 0;
  if (comm.rank() == 0) {
    comm.send(1, tag, size);
    comm.send(1, tag, data.data(), size);
  } else {
    int length;
    comm.recv(0, tag, length);
    data.resize(length);
    comm.recv(0, tag, data.data(), length);
  }
}

// Send a vector using boost::mpi::skeleton, which is exchanging the data
// structure beforehand for better performance.
void send_boost_skeleton(mpi::communicator &comm, std::vector<double> &data) {
  auto size = static_cast<int>(data.size());
  int tag = 0;
  if (comm.rank() == 0) {
    comm.send(1, tag, mpi::skeleton(data));
    comm.send(1, tag, mpi::get_content(data));
  } else {
    int length;
    comm.recv(0, tag, mpi::skeleton(data));
    comm.recv(0, tag, mpi::get_content(data));
  }
}

// Benchmark helper.
template <class Func, class... Args>
double run(const int count, mpi::communicator &comm, const Func &func,
           Args &&... args) {
  comm.barrier();
  const auto start = std::chrono::system_clock::now();
  for (int r = 0; r < count; ++r) {
    func(comm, args...);
  }
  const auto end = std::chrono::system_clock::now();
  comm.barrier();
  std::chrono::duration<double> elapsed = end - start;
  return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed)
             .count() /
         (1000000000);
}

// Benchmark helper.
template <class... Args>
std::pair<double, int> benchmark(mpi::communicator &comm, Args &&... args) {
  // Trial run to compute repeat for running roughly one second
  const auto trial = run(10, comm, args...);
  int repeat = std::max(10, static_cast<int>(1.0 / (trial / 10)));
  mpi::broadcast(comm, repeat, 0);
  return {run(repeat, comm, args...), repeat};
}

// Benchmark helper.
template <class Func>
void benchmark_range(const int min, const int max, const Func &func) {
  mpi::communicator comm;
  int size = min / sizeof(double);

  while (size < max / sizeof(double)) {
    std::vector<double> data(size);

    double seconds;
    int repeat;
    std::tie(seconds, repeat) = benchmark(comm, func, data);

    double kB = static_cast<double>(size * sizeof(double)) / (1024);
    if (comm.rank() == 0)
      printf("%10.3lf kB bandwidth %5.0lf MB/s\n", kB,
             repeat * kB / seconds / 1024);
    size *= 2;
  }
}

int main(int argc, char **argv) {
  mpi::environment env;

  int min_size = 128;
  int max_size = 128 * 1024 * 1024;

  // Four ways to send vector:
  // 1. send_raw_known_size -- fast, but not feasible in practice since we have
  //                           random number of events
  // 2. send_boost -- 2 orders of magnitude slower
  // 3. send_raw_unknown_size -- fast (except for short length vectors)
  // 4. send_boost_skeleton -- fast (except for short/medium length vectors)
  benchmark_range(min_size, max_size, send_raw_unknown_size);

  return 0;
}
