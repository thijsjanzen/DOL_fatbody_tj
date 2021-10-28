//
//  simulation.h
//  dol_fatbody_tj
//
//  Created by Thijs Janzen on 07/04/2021.
//  Copyright © 2021 Thijs Janzen. All rights reserved.
//

#ifndef simulation_h
#define simulation_h

#include <vector>
#include <queue>
#include <tuple>
#include <cassert>
#include <algorithm>

#include "individual.h"
#include "parameters.h"
#include "rand_t.h"

#include <set>

struct track_time {
   ctype_ time;
   individual* ind;

   track_time(individual* input) {
     ind = input;
     time = ind->get_next_t();
   }
};

struct cmp_time {
  bool operator()(const track_time& a, const track_time& b) const {
    return a.time < b.time;
  }
};

struct find_track_time_by_id {
  int focal_id;
  find_track_time_by_id(int x) : focal_id(x) {}
  bool operator()(const track_time& tt) const {
    return tt.ind->get_id() == focal_id;
  }
};

struct Simulation {
  std::vector< individual > colony;

  std::vector<int> nurses;

  std::multiset< track_time, cmp_time > time_queue;

  rnd_t rndgen;

  ctype_ t;
  params p;
  int previous_time_recording;

  ctype_ brood_resources;

  Simulation(const params& par) : p(par) {
    rndgen = rnd_t();
    rndgen.set_threshold_dist(p.mean_threshold, p.sd_threshold);
    colony = std::vector< individual >(p.colony_size);

    t = 0.0;
    previous_time_recording = -1;

    brood_resources = 0.0;

    for (size_t i = 0; i < p.colony_size; ++i) {
      colony[i].set_params(p, static_cast<int>(i), rndgen);

      ctype_ next_t = colony[i].get_next_t_threshold(t, rndgen);
      if(next_t < 0) {
        next_t = 0.0;
      }
      colony[i].go_nurse(next_t);
      nurses.push_back(colony[i].get_id());

      colony[i].update_tasks(t);
      add_to_timequeue(&colony[i]);
    }
  }

  bool check_time_interval(ctype_ t, ctype_ new_t, int time_interval) const {
     if (time_interval <= 0) {
       return false;
     }

    int dt = static_cast<int>(new_t - t);
    if (dt / time_interval > 0) {
      return true;
    } else {
      return false;
    }
   }

   void write_intermediate_output_to_file(std::string file_name,
                                       int t) {
     if (t == previous_time_recording) {
       return;
     }

     if (previous_time_recording == -1) { // first time writing to file
       std::ofstream out_file2(file_name.c_str());
       out_file2 << "time" << "\t"
                << "id" << "\t"
                << "task" << "\t"
                << "fat_body" << "\t"
                << "crop" << "\n";
       out_file2.close();
     }

     previous_time_recording = t;

     std::ofstream out_file(file_name.c_str(), std::ios::app);
     for (const auto& i : colony) {
       out_file << t << "\t"
                << i.get_id() << "\t"
                << static_cast<int>(i.get_task()) << "\t"
                << i.get_fat_body() << "\t"
                << i.get_crop() << "\n";
     }
     out_file.close();
   }

  void remove_from_nurses(int id) {
    for (auto& i : nurses) {
      if (i == id) {
        i = nurses.back();
        nurses.pop_back();
        return;
      }
    }
    std::cout << "ERROR in remove_from_nurses";
    return;
  }

  ctype_ dominance_interaction(ctype_ fb_self, ctype_ fb_other, ctype_ b) {
    //return static_cast<ctype_>(1.f / (1.f + expf(static_cast<float>(fb_self - fb_other))));

    // simple case:
  //  return fb_other / (fb_self + fb_other);

    // soft max
    float exp_other = expf(fb_other * b);
    float exp_self  = expf(fb_self * b);

    return exp_other / (exp_self + exp_other);
  }

  void update_queue(int index) {
    auto it = std::find_if(time_queue.begin(), time_queue.end(),
                            [&](const auto& focal) {return focal.ind->get_id() == index ? true : false; });

    if (it == time_queue.end()) {
      throw(std::runtime_error(std::string("failed to do find_if")));
    }
    time_queue.erase(it);
    add_to_timequeue(&colony[index]);
  }

  void share_resources(individual* focal_individual) {
    if (nurses.empty()) return;

    if (p.model_type > 0 || p.forager_sharing_at_default > 0.f) {
      // in model 0, there is NO sharing
      size_t num_interactions = std::min( static_cast<size_t>(p.max_number_interactions),
                                          static_cast<size_t>(nurses.size()));

      std::vector< int > visited_nurses(num_interactions);

      for (size_t i = 0; i < num_interactions; ++i) {
        if (nurses.size() > 1) {
          size_t j = i + rndgen.random_number(static_cast<int>(nurses.size() - i));
          if (i != j) {
            std::swap(nurses[i], nurses[j]);
          }
        }

        int index_other_individual = nurses[i];

        ctype_ share_amount = p.forager_sharing_at_default;

        switch(p.model_type) {
          case 1: {
              share_amount = 1.f / num_interactions;
            break;
          }
          case 2: {
            share_amount = dominance_interaction(focal_individual->get_dominance()   ,
                                                 colony[index_other_individual].get_dominance(),
                                                 p.soft_max);
            break;
          }
          case 3: {
              share_amount = dominance_interaction(focal_individual->get_fat_body(),
                                                   colony[index_other_individual].get_fat_body(),
                                                   p.soft_max);
            break;
          }
          default: {
            share_amount = 0.f;
          }
        }

        ctype_ to_share = share_amount * focal_individual->get_crop();

        ctype_ food_remaining = colony[index_other_individual].handle_food(to_share,
                                                                          p.max_crop_size,
                                                                          t,
                                                                          p.food_handling_time);

        colony[index_other_individual].process_crop_nurse(p.proportion_fat_body_nurse,
                                                          p.max_fat_body,
                                                          brood_resources);

        visited_nurses[i] = colony[index_other_individual].get_id();

        focal_individual->reduce_crop(to_share - food_remaining);
      }

      for (auto nurse_id : visited_nurses) {
        remove_from_nurses(nurse_id);
        update_queue(nurse_id);
      }
    }
  }

  void update_forager(individual* focal_individual) {
    focal_individual->update_fatbody(t);

    focal_individual->set_crop(p.resource_amount);
    focal_individual->process_crop_forager(p.proportion_fat_body_forager,
                                           p.max_fat_body);

    share_resources(focal_individual);
    // the remainder in the crop is digested entirely.
    focal_individual->process_crop_forager(1.f,
                                           p.max_fat_body);
    return;
  }

  void update_nurse(individual* focal_individual) {
    focal_individual->update_fatbody(t);
    return;
  }

  void pick_task(individual* focal_individual) {
    if (focal_individual->get_previous_task() == task::forage) {
      // individual has returned from foraging
      // now has to decide if he goes foraging again.
      // the moment he goes foraging is picked with new_t:
      focal_individual->decide_new_task(t,
                                       rndgen,
                                       p.foraging_time);
    }
    if (focal_individual->get_previous_task() == task::nurse ||
        focal_individual->get_previous_task() == task::food_handling) {

      if ((focal_individual->get_fat_body() - focal_individual->get_threshold()) < 1e-2f) {



        // individual is here because he has reached his threshold,
        // and goes foraging
        focal_individual->go_forage(t, p.foraging_time);
     } else {
        focal_individual->decide_new_task(t,
                                          rndgen,
                                          p.foraging_time);
      }
    }
  }

  void update_nurse_list(individual* focal_individual) {
    // individual started nursing:
    if (focal_individual->get_task() == task::nurse) {
      // but was not nursing:
      if (focal_individual->get_previous_task() != task::nurse) {
        nurses.push_back(focal_individual->get_id());
      }
    } else {
      // individual stopped nursing
      if (focal_individual->get_previous_task() == task::nurse) {
        // but was nursing
        remove_from_nurses(focal_individual->get_id());
      }
    }
  }

  void add_to_timequeue(individual* focal_individual) {
    assert(focal_individual->get_next_t() >= t);
    time_queue.insert(track_time(focal_individual));
  }


  void update_colony() {

   // assert(is_in_order(time_queue));
    auto focal_individual = time_queue.begin()->ind;
    time_queue.erase(time_queue.begin());


    ctype_ new_t = focal_individual->get_next_t();

    assert(new_t >= t);
    t = new_t;

    // update focal individual
    focal_individual->set_previous_task();

    if (focal_individual->get_task() == task::forage) {
      // update forager
      update_forager(focal_individual);
    } else {
      // nurse done nursing, or done handling food
      update_nurse(focal_individual);
    }

    pick_task(focal_individual);
    update_nurse_list(focal_individual);

    focal_individual->update_tasks(t);

    add_to_timequeue(focal_individual);
  }

  void run_simulation() {

    while(t < p.simulation_time) {
      update_colony();
      assert(colony.size() == time_queue.size());
    }
    // end roll call:
    for (size_t i = 0; i < colony.size(); ++i) {
      colony[i].update_tasks(t);
    }
  }

  

  
};

#endif /* simulation_h */
