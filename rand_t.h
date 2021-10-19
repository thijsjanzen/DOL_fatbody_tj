#ifndef RANDOM_THIJS
#define RANDOM_THIJS
#include <random>
#include <chrono>
#include <thread>
#include <array>

struct rnd_t {
  std::mt19937 rndgen;

  rnd_t() {
    const auto seed_array = make_seed_array();
    std::seed_seq sseq(seed_array.cbegin(), seed_array.cend());

    rndgen = std::mt19937(sseq);
  }

  rnd_t(size_t seed) {
    std::mt19937 rndgen_t(seed);
    rndgen = rndgen_t;
  }

  auto make_seed_array() -> std::array< uint64_t, 5>
  {
    const auto e1 = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
      // with some luck, this is non-deterministic: TRNG
    const auto e2 = static_cast<uint64_t>(std::random_device{}());
    // different between invocations from different threads within one app: thread-id
    const auto tid = std::this_thread::get_id();
    const uint64_t e3{ std::hash<std::remove_const_t<decltype(tid)>>()(tid) };
    // different between installations: compile time macros
    const auto e4 = static_cast<uint64_t>(std::hash<const char*>()(__DATE__ __TIME__ __FILE__));
    // likely different between runs, invocations and platforms: address of local
    //  const auto e5 = reinterpret_cast<uint64_t>(&e1);
    return {{e1, e2, e3, e4, e4 ^ ~0ull /*e5*/}};
  }

  int get_seed() {
    const auto tt = static_cast<int64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    auto tid = std::this_thread::get_id();
    const uint64_t e3{ std::hash<std::remove_const_t<decltype(tid)>>()(tid) };
    auto output = static_cast<int>(tt + e3);
    if (output < 0) output *= -1;
    output += std::random_device{}();
    return output;
  }

  int random_number(int n)    {
    if(n <= 1) return 0;
    return std::uniform_int_distribution<> (0, static_cast<int>(n - 1))(rndgen);
  }

  double normal(double m, double s) {
    std::normal_distribution<double> norm_dist(m, s);
    return norm_dist(rndgen);
  }

  double threshold_normal() {
    double output = threshold_dist(rndgen);
    while(output < 0) output = threshold_dist(rndgen);
    return output;
  }

  void set_threshold_dist(double m, double s) {
    threshold_dist = std::normal_distribution<double>(m, s);
  }

private:
  std::normal_distribution<double> threshold_dist;
};


#endif /* rand_t.h */
