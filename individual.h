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
          // 0       1          2           3
enum class task {nurse, forage, food_handling, max_task};

struct data_storage {
  const ctype_ t_;
  const ctype_ fb_;
  const task current_task_;


  data_storage(ctype_ t, task ct, ctype_ fb) : t_(t), fb_(fb), current_task_(ct)  {}
};



struct individual {
private:
  ctype_ fat_body;
  ctype_ crop;
  ctype_ previous_t;
  ctype_ next_t;
  ctype_ dominance;

  bool is_food_handling;

  ctype_ threshold;

  int ID;

  std::array<ctype_, static_cast<int>(task::max_task)> metabolic_rate;
  task current_task;
  task previous_task;
  std::vector< data_storage > data;

public:

  // delete the copy and move operators:
  // individuals stay in the colony vector and *don't* move
  individual(individual&&) = delete;
  const individual& operator=(individual&&) = delete;
  individual(const individual&) = delete;
  const individual& operator=(const individual&) = delete;

  void new_next_t(ctype_ nt) {
    next_t = nt;
  }

  ctype_ get_next_t_threshold(ctype_ t, rnd_t& rndgen) {
    // this function is only used by nurses
    threshold = static_cast<ctype_>(rndgen.threshold_normal());

    ctype_ dt = metabolic_rate[ static_cast<int>(task::nurse) ] == 0.f ? 1e20f : (fat_body - threshold) / metabolic_rate[ static_cast<int>(task::nurse) ];

    return(t + dt);
  }

  void go_forage(ctype_ t, ctype_ forage_time) {
    current_task = task::forage;
    new_next_t(t + forage_time);
  }

  void go_nurse(ctype_ new_t) {
    current_task = task::nurse;
    new_next_t(new_t);
  }

  void set_previous_task() {
    previous_task = current_task;
  }

  individual() {
    current_task = task::nurse;
    previous_task = task::nurse;
    crop = 0.f;
    fat_body = 1.f;
    is_food_handling = false;
    previous_t = 0.f;
    next_t = 0.f;
    ID = 0;
    dominance = 0.f;
    threshold = 5.f;
    metabolic_rate = {0.0, 0.0, 0.0}; // bogus values
  }

  void update_fatbody(ctype_ t) {
    ctype_ dt = t - previous_t;
    assert(dt >= 0);
    if (dt < 0) return;
    previous_t = t;
  //  assert(current_task < metabolic_rate.size());
    fat_body -= dt * metabolic_rate[ static_cast<int>(current_task) ];
    if (fat_body < 0) fat_body = 0.f; // should not happen!
  }

  void set_params(const params& p,
                  int id,
                  rnd_t& rndgen) {
    fat_body = p.init_fat_body;

    metabolic_rate = {p.metabolic_cost_nurses,
                      p.metabolic_cost_foragers,
                      p.metabolic_cost_nurses};

    ID = id;
    dominance = static_cast<ctype_>(rndgen.normal(p.mean_dominance, p.sd_dominance));
  }

  void process_crop_forager(ctype_ fraction, ctype_ max_fat_body) {
    ctype_ processed = crop * fraction;

    fat_body += processed;
    crop -= processed;
    if (fat_body > max_fat_body) fat_body = max_fat_body;
  }

  void process_crop_nurse(ctype_ fraction,
                          ctype_ max_fat_body,
                          ctype_& brood_resources) {
    ctype_ processed = crop * fraction;
    if (fat_body + processed < max_fat_body) {
      fat_body += processed;
      crop -= processed;
      brood_resources += crop;
      crop = 0.f;
    } else {
      processed = max_fat_body - fat_body;

      fat_body += processed;
      crop -= processed;
      brood_resources += crop;
      crop = 0.f;
    }
  }

  void reduce_crop(ctype_ amount) {
    crop -= amount;
    if (crop < 0.f) crop = 0.f;
  }

  ctype_ handle_food(ctype_ food, // amount shared by the forager to the nurse
                     ctype_ max_crop_size,
                     ctype_ t,
                     ctype_ handling_time) {

    // code below can be shorter, but now shows better the decision tree.
    if (crop + food < max_crop_size) {
      crop += food;
      food -= food;
    } else {
      ctype_ food_received = max_crop_size - crop;
      crop += food_received;
      food -= food_received;
    }

    current_task = task::food_handling;
    new_next_t(t + handling_time);
    is_food_handling = true;

    return food;
  }

  void decide_new_task(ctype_ t,
                      rnd_t& rndgen,
                      ctype_ foraging_time) {
    is_food_handling = false;
    ctype_ new_t = get_next_t_threshold(t, rndgen);

    // if new_t is in the future: go nursing
    // otherwise, go foraging
     if (new_t <= t) {
     // individual goes foraging
       go_forage(t, foraging_time);
     } else {
       go_nurse(new_t);
     }
  }


  void update_tasks(ctype_ t) {
    assert(t >= previous_t);
    previous_t = t;

    auto focal_task = current_task;
    if (current_task == task::food_handling) focal_task = task::nurse;

    data.push_back( data_storage(t, focal_task, fat_body));
  }

  double calc_freq_switches(ctype_ min_t, ctype_ max_t) const {
    if (data.size() <= 1) {
      return 0.0;
    }

    size_t cnt = 0;
    size_t checked_time_points = 0;
    for (size_t i = 1; i < data.size(); ++i) {

      ctype_ t1 = data[i - 1].t_;
      ctype_ t2 = data[i].t_;
      if (t1 >= min_t && t2 <= max_t) {
        checked_time_points++;
        auto task1 = data[i].current_task_;
        auto task2 = data[i - 1].current_task_;

        if (task1 != task2) {
          cnt++;
        }
      }
    }

    return cnt * 1.0 / checked_time_points;;
  }

  size_t count_p(ctype_ min_t, ctype_ max_t,
                 size_t& num_switches) const {
    if (data.size() <= 1) {
      num_switches += 1;
      return 0;
    }

    size_t cnt = 0;
    for (const auto& i : data) {
      ctype_ t = i.t_;
      if (t >= min_t && t <= max_t) {
        if (i.current_task_ == task::nurse) cnt++;
        num_switches++;
      }
    }
    return cnt;
  }

  std::vector<ctype_> calculate_task_frequency(ctype_ min_t, ctype_ max_t) const {
    std::vector<ctype_> task_freq(2, 0.0);

    for (size_t i = 0; i < data.size(); ++i) {

      ctype_ start_t = data[i].t_;
      ctype_ end_t = max_t;
      if (i + 1 < data.size()) {
        end_t = data[i + 1].t_;
      }

      if (start_t >= min_t && end_t <= max_t &&
          start_t <= max_t && end_t >= min_t) {
        ctype_ dt = end_t - start_t;
        int index = static_cast<int>(data[i].current_task_);
        assert(dt >= 0.f);
        task_freq[ index ] += dt;
      }
    }
    return task_freq;
  }


  ctype_ get_fat_body() const {return fat_body;}
  ctype_ get_dominance() const {return dominance;}
  ctype_ get_crop() const {return crop;}
  ctype_ get_previous_t() const {return previous_t;}
  ctype_ get_next_t() const {return next_t;}
  ctype_ get_threshold() const {return threshold;}
  task get_task() const {return current_task;}
  int get_id() const {return ID;}
  task get_previous_task() const {return previous_task;}
  const std::vector< data_storage >& get_data() const {return data;}

  void set_fat_body(ctype_ fb) {fat_body = fb;}
  void set_crop(ctype_ c) {
    crop = c;
  }
  void set_previous_t(ctype_ t) {previous_t = t;}
  void set_current_task(task new_task) {current_task = new_task;}
  void set_is_food_handling(bool new_val) {is_food_handling = new_val;}

  bool operator==(int other_id) {
    if (ID == other_id) return true;
    return false;
  }
};


#endif /* individual_h */
