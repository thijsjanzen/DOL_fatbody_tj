//
//  individual.h
//  dol_fatbody_tj
//
//  Created by Thijs Janzen on 06/04/2021.
//  Copyright © 2021 Thijs Janzen. All rights reserved.
//

#ifndef individual_h
#define individual_h

#include "parameters.h"
#include "rand_t.h"
#include <cassert>

enum task {nurse, forage, food_handling};

struct individual {
private:
  float fat_body;
  float crop;
  float max_crop_size;
  float previous_t;
  float next_t;
  float dominance;

  bool is_food_handling;

  float threshold;

  int ID;

  std::vector<float> metabolic_rate;
  task current_task;
  task previous_task;
  std::vector< std::tuple<float, task, float > > data;

public:

  // delete the copy and move operators:
  // individuals stay in the colony vector and *don't* move
  individual(individual&&) = delete;
  const individual& operator=(individual&&) = delete;
  individual(const individual&) = delete;
  const individual& operator=(const individual&) = delete;

  void new_next_t(double nt) {
 //   assert(nt > next_t);
    next_t = nt;
  }

  double get_next_t_threshold(float t, rnd_t& rndgen) {
    threshold = rndgen.threshold_normal();
    double dt = (fat_body - threshold) / metabolic_rate[ nurse ];
 
    return(t + dt);
  }

  void go_forage(float t, float forage_time) {
    current_task = forage;
    new_next_t(t + forage_time);
  }

  void go_nurse(float new_t) {
    current_task = nurse;
    new_next_t(new_t);
  }

  void set_previous_task() {
    previous_task = current_task;
  }

  individual() {
    current_task = nurse;
    previous_task = nurse;
    crop = 0.f;
    fat_body = 1.f;
    is_food_handling = false;
    previous_t = 0.f;
    next_t = 0.f;
  }

  void update_fatbody(float t) {
    float dt = t - previous_t;
    assert(dt >= 0);
    if (dt < 0) return;
    previous_t = t;
    fat_body -= dt * metabolic_rate[ current_task ];
    if (fat_body < 0) fat_body = 0.f; // should not happen!
  }

  void set_params(const params& p,
                  int id,
                  rnd_t& rndgen) {
    fat_body = p.init_fat_body;

    metabolic_rate = std::vector<float>{p.metabolic_cost_nurses,
                                      p.metabolic_cost_foragers};
    ID = id;
    max_crop_size = p.crop_size;
    dominance = rndgen.normal(p.mean_dominance, p.sd_dominance);
  }

  void process_crop(float fraction, float max_fat_body) {
    float processed = crop * fraction;

    fat_body += processed;
    crop -= processed;
    if (fat_body > max_fat_body) fat_body = max_fat_body;
  }

  void reduce_crop(float amount) {
    crop -= amount;
    if (crop < 0.f) crop = 0.f;
  }

  void eat_crop(float max_fat_body) {
    fat_body += crop;
    crop = 0.f;
    if (fat_body > max_fat_body) fat_body = max_fat_body;
  }

  double handle_food(float food,
                    float conversion_rate,
                    float max_fat_body,
                    float t,
                    float handling_time) {
    if ((fat_body + (food * conversion_rate)) > max_fat_body) {

      float uptake = (max_fat_body - fat_body) / conversion_rate;

      food -= uptake;
      fat_body = max_fat_body;
    } else {
      fat_body += food * conversion_rate;
      food = 0.f; // all the shared food is gone
    }

    current_task = food_handling;
    new_next_t(t + handling_time);
    is_food_handling = true;

    return food;
  }

  void decide_new_task(float t,
                      rnd_t& rndgen,
                      float foraging_time) {
    is_food_handling = false;
    double new_t = get_next_t_threshold(t, rndgen);

    // if new_t is in the future: go nursing
    // otherwise, go foraging
     if (new_t <= t) {
     // individual goes foraging
       go_forage(t, foraging_time);
     } else {
       go_nurse(new_t);
     }
  }


  void update_tasks(float t) {
    assert(t >= previous_t);
    previous_t = t;

    auto focal_task = current_task;
    if (current_task == food_handling) focal_task = nurse;

    data.push_back( std::make_tuple(t, focal_task, fat_body));
  }

  double calc_freq_switches() const {
    if (data.size() <= 1) {
      return 0.0;
    }

    int cnt = 0;
    for (size_t i = 1; i < data.size(); ++i) {
      auto task1 = std::get<1>(data[i]);
      auto task2 = std::get<1>(data[i - 1]);


      if (task1 != task2) {
        cnt++;
      }
    }
    double freq_switches = cnt * 1.0 / (data.size() - 1);

    return freq_switches;
  }

  double count_p(size_t& num_switches) const {
    if (data.size() <= 1) {
      num_switches += 1;
      return 0.0;
    }

    int cnt = 0;
    for (const auto& i : data) {
      if (std::get<1>(i) == 0) cnt++;
      num_switches++;
    }
    return cnt;
  }

  std::vector<double> calculate_task_frequency(double total_runtime) {
    std::vector<double> task_freq(3, 0.0);

    for (size_t i = 0; i < data.size(); ++i) {
      float start_t = std::get<0>(data[i]);
      float end_t = total_runtime;
      if (i + 1 < data.size()) {
        end_t = std::get<0>(data[i + 1]);
      }
      
      double dt = end_t - start_t;
      task_freq[ std::get<1>(data[i]) ] += dt;
    }
    return task_freq;
  }


  float get_fat_body() const {return fat_body;}
  float get_dominance() const {return dominance;}
  float get_crop() const {return crop;}
  float get_previous_t() const {return previous_t;}
  float get_next_t() const {return next_t;}
  float get_threshold() const {return threshold;}
  task get_task() const {return current_task;}
  int get_id() const {return ID;}
  task get_previous_task() const {return previous_task;}
  const std::vector< std::tuple<float, task, float > >& get_data() const {return data;}

  void set_fat_body(float fb) {fat_body = fb;}
  void set_crop(float c) {
    crop = c;
  }
  void set_previous_t(float t) {previous_t = t;}
  void set_current_task(task new_task) {current_task = new_task;}

  bool operator==(int other_id) {
    if (ID == other_id) return true;
    return false;
  }
};


#endif /* individual_h */
