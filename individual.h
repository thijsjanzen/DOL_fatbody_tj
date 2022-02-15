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
#include <limits>


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
  ctype_ threshold;
  ctype_ max_fat_body;

  std::array<ctype_, static_cast<int>(task::max_task)> metabolic_rate;
  task current_task;
  task previous_task;
  std::vector< data_storage > data;

  std::vector<ctype_> (*share_interaction_grouped)(individual*, std::vector<individual*>, ctype_, size_t);

public:

  // delete the copy and move operators:
  // individuals stay in the colony vector and *don't* move
  individual(individual&&) = delete;
  const individual& operator=(individual&&) = delete;
  individual(const individual&) = delete;
  const individual& operator=(const individual&) = delete;

  void initialize(const params& p,
                  rnd_t& rndgen,
                  std::vector<ctype_> (*share_func_grouped)(individual*, std::vector<individual*>, ctype_, size_t)) {
    share_interaction_grouped = share_func_grouped;

    fat_body = p.init_fat_body;
    max_fat_body = p.max_fat_body;

    metabolic_rate = {p.metabolic_cost_nurses,
                      p.metabolic_cost_foragers,
                      p.metabolic_cost_nurses};

    dominance = static_cast<ctype_>(rndgen.uniform());
    next_t = get_next_t_threshold(ctype_(0.0), rndgen);
    if(next_t < 0.0) {
      next_t = ctype_(0.0);
    }

    start_task(next_t, task::nurse);

    update_data(ctype_(0.0));
  }

  individual() {
    current_task = task::nurse;
    previous_task = task::nurse;
    crop = 0.f;
    fat_body = 1.f;
    previous_t = 0.f;
    next_t = 0.f;
    dominance = 0.f;
    threshold = 5.f;
    metabolic_rate = {1.0, 1.0, 1.0}; // bogus values
  }

  void update(ctype_ t,
              const params& p,
              rnd_t& rndgen,
              std::vector< individual* >& nurses) {

    set_previous_task(); 

    if (previous_task == task::forage) {
      // update forager
      update_forager(t, p, nurses, rndgen);
    } else {
      // nurse done nursing, or done handling food
      update_nurse(t);
    }

    pick_new_task(t, rndgen, p);

    update_data(t);
  }

  ctype_ get_next_t_threshold(ctype_ t, rnd_t& rndgen) {
    // this function is only used by nurses
    threshold = static_cast<ctype_>(rndgen.threshold_normal());
    ctype_ dt = metabolic_rate[ static_cast<int>(task::nurse) ] == 0.f ? 1e20f : (fat_body - threshold) / metabolic_rate[ static_cast<int>(task::nurse) ];

    return(t + dt);
  }

  void start_task(ctype_ new_t, task new_task) {
    current_task = new_task;
    next_t = new_t;
  }

  void set_previous_task() {
    previous_task = current_task;
  }

  void update_fatbody(ctype_ t) {
    ctype_ dt = t - previous_t;
    assert(dt >= 0);
    if (dt < 0) return;
    previous_t = t;
    fat_body -= dt * metabolic_rate[ static_cast<int>(current_task) ];
    if (fat_body < 0) fat_body = 0.f; // should not happen!
  }

  void process_crop() {
    fat_body += crop;
    crop = 0.f;
    if (fat_body > max_fat_body) fat_body = max_fat_body;
  }

  void reduce_crop(ctype_ amount) {
    crop -= amount;
    if (crop < 0.f) crop = 0.f;
  }

  ctype_ handle_food(ctype_ food, // amount shared by the forager to the nurse
                     ctype_ t,
                     ctype_ handling_time) {

    crop += food;
    food -= food;

    current_task = task::food_handling;
    next_t = t + handling_time;

    return food;
  }

  void decide_new_task(ctype_ t,
                       rnd_t& rndgen,
                       ctype_ foraging_time) {
    ctype_ new_t = get_next_t_threshold(t, rndgen);

    // if new_t is in the future: go nurse
    // otherwise, go foraging
     if (new_t <= t) {
     // individual goes foraging immediately
       start_task(t + foraging_time, task::forage);
     } else {
       start_task(new_t, task::nurse);
     }
  }

  void update_data(ctype_ t) {
    assert(t >= previous_t);
    previous_t = t;

    auto focal_task = current_task;
    if (current_task == task::food_handling) focal_task = task::nurse;

    data.push_back( data_storage(t, focal_task, fat_body));
  }

  void pick_new_task(ctype_ t,
                     rnd_t& rndgen,
                     const params& p) {
    if (get_previous_task() == task::forage) {
      // individual has returned from foraging
      // now has to decide if he goes foraging again.
      // the moment he goes foraging is picked with new_t:
      decide_new_task(t,
                      rndgen,
                      p.foraging_time);
    } else { // nursing or food handling
      if (fat_body - threshold < ctype_(1e-2) ) {
        // individual is here because he has reached his threshold,
        // and goes foraging
        start_task(t + p.foraging_time, task::forage);
     } else {
        decide_new_task(t,
                        rndgen,
                        p.foraging_time);
      }
    }
  }

  void update_forager(ctype_ t,
                      const params& p,
                      std::vector< individual* >& nurses,
                      rnd_t& rndgen) {
    update_fatbody(t);

    set_crop(p.resource_amount);
    share_resources_grouped(t, nurses, p, rndgen);
    // the remainder in the crop is digested entirely.
    process_crop();
    return;
  }

  void update_nurse(ctype_ t) {
    update_fatbody(t);
    return;
  }


  ctype_ get_fat_body() const {return fat_body;}
  ctype_ get_relative_fat_body() const {return fat_body * 1.0 / max_fat_body;}
  ctype_ get_dominance() const {return dominance;}
  ctype_ get_crop() const {return crop;}
  ctype_ get_previous_t() const {return previous_t;}
  ctype_ get_next_t() const {return next_t;}
  ctype_ get_threshold() const {return threshold;}
  task get_task() const {return current_task;}
  task get_previous_task() const {return previous_task;}
  const std::vector< data_storage >& get_data() const {return data;}

  void set_fat_body(ctype_ fb) {fat_body = fb;}
  void set_crop(ctype_ c) { crop = c;}
  void set_previous_t(ctype_ t) {previous_t = t;}
  void set_current_task(task new_task) {current_task = new_task;}
  void set_dominance(ctype_ d) {dominance = d;} // for testing


  void share_resources_grouped(ctype_ t,
                               std::vector< individual* >& nurses,
                               const params& p,
                               rnd_t& rndgen) {

    if (nurses.empty()) return;

    size_t num_interactions = std::min( static_cast<size_t>(p.max_number_interactions),
                                         static_cast<size_t>(nurses.size()));

    for (size_t i = 0; i < num_interactions; ++i) {
      if (nurses.size() > 1) {
        size_t j = i + rndgen.random_number(static_cast<int>(nurses.size() - i));
        if (i != j) {
          std::swap(nurses[i], nurses[j]);
        }
      }
      // now, we have selected num_interactions nurses randomly
    }

    std::vector< ctype_ > share_amount = share_interaction_grouped(this,
                                                                   nurses,
                                                                   p.soft_max,
                                                                   num_interactions);

    ctype_ total_crop = this->get_crop();
    for (size_t i = 0; i < num_interactions; ++i) {

        ctype_ to_share = share_amount[i] * total_crop;

      if (to_share > 0.0) {

        ctype_ food_remaining = nurses[i]->handle_food(to_share,
                                                       t,
                                                       p.food_handling_time);

        nurses[i]->process_crop();

        this->reduce_crop(to_share - food_remaining);
      }
    }
  }
};

inline ctype_ get_exp(ctype_ val) {
  static ctype_ max_val = log(std::numeric_limits<ctype_>::max());
  if (val > max_val) {
    return std::numeric_limits<ctype_>::max();
  }

  return std::exp(val);
}

std::vector<ctype_> no_sharing_grouped(individual* pivot,
                          std::vector<individual*> other,
                          ctype_ soft_max,
                          size_t num_interactions)  {
  return std::vector<ctype_>(num_interactions, ctype_(0.0));
}

std::vector<ctype_> fair_sharing_grouped(individual* pivot,
                            std::vector<individual*> other,
                            ctype_ soft_max,
                            size_t num_interactions)  {
  return std::vector<ctype_>(num_interactions, ctype_(1) / ( 1 + num_interactions)); // 1 + for forager
}

std::vector<ctype_> dominance_sharing_grouped(individual* pivot,
                                              std::vector<individual*> other,
                                              ctype_ soft_max,
                                              size_t num_interactions)  {

  std::vector<ctype_> share(num_interactions);

  ctype_ sum = std::exp(pivot->get_dominance() * soft_max);

  for (size_t i = 0; i < num_interactions; ++i) {
    share[i] = std::exp( other[i]->get_dominance() * soft_max);

    sum += share[i];
  }
  sum = ctype_(1) / sum;
  for (auto& i : share) {
    i *= sum;
  }

  return share;
}

std::vector<ctype_> fatbody_sharing_grouped(individual* pivot,
                                            std::vector<individual*> other,
                                            ctype_ soft_max,
                                            size_t num_interactions)  {
  std::vector<ctype_> share(num_interactions);
  ctype_ sum = get_exp(pivot->get_relative_fat_body()  * soft_max);
  for (size_t i = 0; i < num_interactions; ++i) {
    share[i] = get_exp(other[i]->get_relative_fat_body() * soft_max);
    sum += share[i];
  }

  sum = ctype_(1) / sum;

  for (auto& i : share) {
    i *= sum;
  }

  return share;
}


#endif /* individual_h */
