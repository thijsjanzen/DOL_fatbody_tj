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

// TODO: make general interaction functor/thing/stuff
ctype_ dominance_interaction(ctype_ fb_self, ctype_ fb_other, ctype_ b) {
    // soft max
    float exp_other = expf(fb_other * b);
    float exp_self  = expf(fb_self * b);

    return exp_other / (exp_self + exp_other);
}

struct individual {
private:
  ctype_ fat_body;
  ctype_ crop;
  ctype_ previous_t;
  ctype_ next_t;
  ctype_ dominance;
  ctype_ threshold;

  std::array<ctype_, static_cast<int>(task::max_task)> metabolic_rate;
  task current_task;
  task previous_task;
  std::vector< data_storage > data;

  ctype_ (*share_interaction)(individual*, individual*, ctype_, size_t);

public:

  // delete the copy and move operators:
  // individuals stay in the colony vector and *don't* move
  individual(individual&&) = delete;
  const individual& operator=(individual&&) = delete;
  individual(const individual&) = delete;
  const individual& operator=(const individual&) = delete;

  void initialize(const params& p,
                  rnd_t& rndgen,
                  ctype_ (*share_func)(individual*, individual*, ctype_, size_t)) {
    fat_body = p.init_fat_body;

    metabolic_rate = {p.metabolic_cost_nurses,
                      p.metabolic_cost_foragers,
                      p.metabolic_cost_nurses};

    dominance = static_cast<ctype_>(rndgen.uniform());
    next_t = get_next_t_threshold(ctype_(0.0), rndgen);
    if(next_t < 0.0) {
      next_t = ctype_(0.0);
    }
    go_nurse(next_t);

    update_tasks(ctype_(0.0));
    share_interaction = share_func;
  }

  void update(ctype_ t,
              const params& p,
              rnd_t& rndgen,
              std::vector< individual* > nurses) {
    set_previous_task();

    if (current_task == task::forage) {
      // update forager
      update_forager(t, p, nurses, rndgen);
    } else {
      // nurse done nursing, or done handling food
      update_nurse(t);
    }

    pick_task(t, rndgen, p);

    update_tasks(t);
  }

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
    previous_t = 0.f;
    next_t = 0.f;
    dominance = 0.f;
    threshold = 5.f;
    metabolic_rate = {1.0, 1.0, 1.0}; // bogus values
  }

  void update_fatbody(ctype_ t) {
    ctype_ dt = t - previous_t;
    assert(dt >= 0);
    if (dt < 0) return;
    previous_t = t;
    fat_body -= dt * metabolic_rate[ static_cast<int>(current_task) ];
    if (fat_body < 0) fat_body = 0.f; // should not happen!
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

    return food;
  }

  void decide_new_task(ctype_ t,
                       rnd_t& rndgen,
                       ctype_ foraging_time) {
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

  void pick_task(ctype_ t,
                 rnd_t& rndgen,
                 const params& p) {
    if (get_previous_task() == task::forage) {
      // individual has returned from foraging
      // now has to decide if he goes foraging again.
      // the moment he goes foraging is picked with new_t:
      decide_new_task(t,
                      rndgen,
                      p.foraging_time);
    }
    if (get_previous_task() == task::nurse ||
        get_previous_task() == task::food_handling) {

      if (fat_body - threshold < ctype_(1e-2) ) {
        // individual is here because he has reached his threshold,
        // and goes foraging
        go_forage(t, p.foraging_time);
     } else {
        decide_new_task(t,
                        rndgen,
                        p.foraging_time);
      }
    }
  }

  void update_forager(ctype_ t,
                      const params& p,
                      std::vector< individual* > nurses,
                      rnd_t& rndgen) {
    update_fatbody(t);

    set_crop(p.resource_amount);
    process_crop_forager(p.proportion_fat_body_forager,
                                           p.max_fat_body);

    share_resources(t, nurses, p, rndgen);
    // the remainder in the crop is digested entirely.
    process_crop_forager(ctype_(1.0), p.max_fat_body);
    return;
  }

  void update_nurse(ctype_ t) {
    update_fatbody(t);
    return;
  }


  ctype_ get_fat_body() const {return fat_body;}
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

  void share_resources(ctype_ t,
                       std::vector< individual* > nurses,
                       const params& p,
                       rnd_t& rndgen) {
    if (nurses.empty()) return;

    ctype_ brood_resources = ctype_(0);

      // in model 0, there is NO sharing
    size_t num_interactions = std::min( static_cast<size_t>(p.max_number_interactions),
                                        static_cast<size_t>(nurses.size()));

    for (size_t i = 0; i < num_interactions; ++i) {
      if (nurses.size() > 1) {
        size_t j = i + rndgen.random_number(static_cast<int>(nurses.size() - i));
        if (i != j) {
          std::swap(nurses[i], nurses[j]);
        }
      }

      ctype_ share_amount = p.forager_sharing_at_default;

      share_amount = share_interaction(this,
                                       nurses[i],
                                       p.soft_max,
                                       num_interactions);

      ctype_ to_share = share_amount * crop;

      ctype_ food_remaining = nurses[i]->handle_food(to_share,
                                                     p.max_crop_size,
                                                     t,
                                                     p.food_handling_time);

      nurses[i]->process_crop_nurse(p.proportion_fat_body_nurse,
                                    p.max_fat_body,
                                    brood_resources);

      reduce_crop(to_share - food_remaining);
      nurses[i]->set_current_task(task::food_handling);
    }
  }
};

ctype_ no_sharing(individual* pivot,
                  individual* other,
                  ctype_ soft_max,
                  size_t num_interactions)  {
    return ctype_(0.0);
}

ctype_ fair_sharing(individual* pivot,
                    individual* other,
                    ctype_ soft_max,
                    size_t num_interactions)  {
    return ctype_(1) / num_interactions;
}

ctype_ dominance_sharing(individual* pivot,
                         individual* other,
                         ctype_ soft_max,
                         size_t num_interactions)  {

    ctype_ exp_other = expf(other->get_dominance() * soft_max);
    ctype_ exp_self  = expf(pivot->get_dominance() * soft_max);

    return exp_other / (exp_self + exp_other);
}

ctype_ fatbody_sharing (individual* pivot,
                        individual* other,
                        ctype_ soft_max,
                        size_t num_interactions)  {

    ctype_ exp_other = expf(other->get_fat_body() * soft_max);
    ctype_ exp_self  = expf(pivot->get_fat_body() * soft_max);

    return exp_other / (exp_self + exp_other);
}


#endif /* individual_h */
