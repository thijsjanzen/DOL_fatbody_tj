#ifndef RANDOM_THIJS
#define RANDOM_THIJS
#include <random>
#include <chrono>
#include <thread>

struct rnd_t {
  std::mt19937 rndgen;

  rnd_t() {
    std::mt19937 rndgen_t(get_seed());
    rndgen = rndgen_t;
  }

  rnd_t(size_t seed) {
    std::mt19937 rndgen_t(seed);
    rndgen = rndgen_t;
  }

  int get_seed() {
    const auto tt = static_cast<int64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    auto tid = std::this_thread::get_id();
    const uint64_t e3{ std::hash<std::remove_const_t<decltype(tid)>>()(tid) };
    auto output = static_cast<int>(tt + e3);
    if (output < 0) output *= -1;
    return output;
  }

  std::uniform_real_distribution<float> unif_dist =
    std::uniform_real_distribution<float>(0.0f, 1.0f);

  int random_number(int n)    {
    if(n <= 1) return 0;
    return std::uniform_int_distribution<> (0, static_cast<int>(n - 1))(rndgen);
  }

  float uniform()    {
    return unif_dist(rndgen);
  }

  void set_seed(unsigned seed)    {
    std::mt19937 new_randomizer(seed);
    rndgen = new_randomizer;
  }

  bool bernouilli(double p) {
    std::bernoulli_distribution d(p);
    return(d(rndgen));
  }

  int poisson(double lambda) {
    return std::poisson_distribution<int>(lambda)(rndgen);
  }

  double normal(double m, double s) {
    std::normal_distribution<double> norm_dist(m, s);
    return norm_dist(rndgen);
  }

  double normal_positive(double m, double s) {
    std::normal_distribution<double> norm_dist(m, s);
    double  output = norm_dist(rndgen);
    while(output < 0) output = norm_dist(rndgen);
    return output;
  }

  double threshold_normal() {
    double  output = threshold_dist(rndgen);
    while(output < 0) output = threshold_dist(rndgen);
    return output;
  }

  void set_threshold_dist(double m, double s) {
    threshold_dist = std::normal_distribution<double>(m, s);
  }

private:
  std::poisson_distribution<int> recom_pos_gen;
  std::normal_distribution<double> threshold_dist;

};


#endif /* rand_t.h */
