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
private:
  float fat_body;
  float crop;
  float max_crop_size;
  float previous_t;
  float next_t;
  float half_point;
  float dominance;

  int ID;

  std::vector<float> metabolic_rate;
  task current_task;
  std::vector< std::tuple<float, task, float > > data;

public:
  // delete the copy and move operators:
  // individuals stay in the colony vector and *don't* move
  individual(individual&&) = delete;
  const individual& operator=(individual&&) = delete;
  individual(const individual&) = delete;
  const individual& operator=(const individual&) = delete;


  float forage_prob(float dt) {
    return dt * 1.0f / (1.0f + expf(fat_body - half_point));
  }

  void set_next_t(double t, double foraging_time) {
    next_t = t + foraging_time;
  }

  individual() {
    current_task = nurse;
    crop = 0.0;
    fat_body = 1.0;
  }

  void update_fatbody(float t) {
    float dt = t - previous_t;
    previous_t = t;
    fat_body -= dt * metabolic_rate[ current_task ];
    if (fat_body < 0) fat_body = 0.0; // should not happen!
  }

  void set_params(const ind_param& p,
                  int id,
                  rnd_t& rndgen) {
    fat_body = p.fat_body_size;

    metabolic_rate = std::vector<float>{p.metabolic_cost_nurses,
                                        p.metabolic_cost_foragers};

    half_point = p.half_point;
    ID = id;
    max_crop_size = p.crop_size;
    dominance = rndgen.normal(p.mean_dominance, p.sd_dominance);
    crop = 0.0;
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

  double receive_food(float food,
                      float conversion_rate,
                      float max_fat_body) {
    float prev_fatbody = fat_body;
    if ((fat_body + (food * conversion_rate)) > max_fat_body) {

      float uptake = (max_fat_body - fat_body) / conversion_rate;

      food -= uptake;
      fat_body = max_fat_body;
    } else {
      fat_body += food * conversion_rate;
      food = 0.0; // all the shared food is gone
    }

    return food;
  }

  void update_tasks(float t) {
    previous_t = t;

    if (!data.empty()) {
      float temp_fatbody = std::get<2>(data.back());
      float diff_fb = fat_body - temp_fatbody;
    }

    data.push_back( std::make_tuple(t, current_task, fat_body));
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
    std::vector<double> task_freq(2, 0.0);

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
  task get_task() const {return current_task;}
  int get_id() const {return ID;}
  const std::vector< std::tuple<float, task, float > >& get_data() const {return data;}

  void set_fat_body(float fb) {fat_body = fb;}
  void set_crop(float c) {
    crop = c;
  }
  void set_previous_t(float t) {previous_t = t;}
  void set_current_task(task new_task) {current_task = new_task;}




};


#endif /* individual_h */
