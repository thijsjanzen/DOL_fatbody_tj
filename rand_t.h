#ifndef RANDOM_THIJS
#define RANDOM_THIJS
#include <random>
#include <chrono>
#include <thread>
#include <array>

struct rnd_t {
  std::mt19937 rndgen;

  rnd_t(ctype_ m, ctype_ s) {
    const auto seed_array = make_seed_array();
    std::seed_seq sseq(seed_array.cbegin(), seed_array.cend());

    rndgen = std::mt19937(sseq);
    set_threshold_dist(m, s);
  }

  auto make_seed_array() -> std::array< uint64_t, 5>
  {
    const auto e1 = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    const auto e2 = static_cast<uint64_t>(std::random_device{}());
    // different between invocations from different threads within one app: thread-id
    const auto tid = std::this_thread::get_id();
    const uint64_t e3{ std::hash<std::remove_const_t<decltype(tid)>>()(tid) };
    // different between installations: compile time macros
    const auto e4 = static_cast<uint64_t>(std::hash<const char*>()(__DATE__ __TIME__ __FILE__));
    // likely different between runs, invocations and platforms: address of local
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

  ctype_ normal(ctype_ m, ctype_ s) {
    std::normal_distribution<ctype_> norm_dist(static_cast<ctype_>(m), static_cast<ctype_>(s));
    return norm_dist(rndgen);
  }

  ctype_ uniform() {
    return unif_dist(rndgen);
  }

  ctype_ threshold_normal() {
    ctype_ output = threshold_dist(rndgen);
    while(output < 0) output = threshold_dist(rndgen);
    return output;
  }

  void set_threshold_dist(ctype_ m, ctype_ s) {
    threshold_dist = std::normal_distribution<ctype_>(static_cast<ctype_>(m), static_cast<ctype_>(s));
  }

private:
  std::normal_distribution<float> threshold_dist;
  std::uniform_real_distribution<ctype_> unif_dist = std::uniform_real_distribution<ctype_>(ctype_(0), 
                                                                                      ctype_(1));
};


#endif /* rand_t.h */
