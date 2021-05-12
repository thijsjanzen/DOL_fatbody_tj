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

enum task {nurse, forage};

struct individual {

  // delete the copy and move operators:
  // individuals stay in the colony vector and *don't* move
  individual(individual&&) = delete;
  const individual& operator=(individual&&) = delete;
  individual(const individual&) = delete;
  const individual& operator=(const individual&) = delete;


  individual() {
    previous_t = 0.0;
    current_task = nurse;
    crop = 0.0;
    fat_body = 1.0;
  }

  void update_fatbody(float current_t) {
    fat_body -= (current_t - previous_t) * metabolic_rate[ current_task ];
    if (fat_body < 0) fat_body = 0.0; // should not happen!
    previous_t = current_t;
  }

  void update_threshold(rnd_t& rndgen) {
    threshold = rndgen.threshold_normal();
    set_next_t_nurse();
  }

  void set_next_t_nurse() {
    next_t = previous_t + (fat_body - threshold) / metabolic_rate[ current_task];
  }

  void set_next_t_forager(float foraging_time) {
    next_t = previous_t + foraging_time;
  }

  bool not_at_threshold() {
    if (fat_body - threshold > 1e-3) return true;
    return false;
  }


  void set_params(const ind_param& p, int id) {
    fat_body = p.fat_body_size;

    metabolic_rate = std::vector<float>{p.metabolic_cost_nurses,
                                        p.metabolic_cost_foragers};
    ID = id;
    max_crop_size = p.crop_size;
  }

  void process_crop(float fraction) {
    float processed = crop * fraction;
    fat_body += processed;
    crop -= processed;
  }

  void reduce_crop(float amount) {
    crop -= amount;
    if (crop < 0.0) crop = 0.0;
  }

  void receive_food(float food, float conversion_rate) {
    fat_body += food * conversion_rate;
  }

  void update_tasks(float t) {
    previous_t = t;
    data.push_back( std::make_tuple(previous_t, current_task, fat_body));
  }

  double calc_freq_switches() const {
    if (data.size() <= 1) {
      return 0.0;
    }

    int cnt = 0;
    for (int i = 1; i < data.size(); ++i) {
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


  float get_fat_body() const {return fat_body;}
  float get_crop() const {return crop;}
  float get_previous_t() const {return previous_t;}
  float get_next_t() const {return next_t;}
  task get_task() const {return current_task;}
  int get_id() const {return ID;}
  const std::vector< std::tuple<float, task, float > >& get_data() const {return data;}

  void set_fat_body(float fb) {fat_body = fb;}
  void set_crop(float c) {crop = c;}
  void set_previous_t(float t) {previous_t = t;}
  void set_current_task(task new_task) {current_task = new_task;}

private:
  float fat_body;
  float crop;
  float max_crop_size;
  float previous_t;
  float next_t;
  float threshold;

  int ID;

  std::vector<float> metabolic_rate;
  task current_task;
  std::vector< std::tuple<float, task, float > > data;
};


#endif /* individual_h */