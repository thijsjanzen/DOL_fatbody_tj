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
#include <limits>

#include "individual.h"
#include "parameters.h"
#include "rand_t.h"

#include <set>

struct Simulation {
  std::vector< individual > colony;
  std::vector< individual* > nurses;

  params p;
  rnd_t rndgen;

  ctype_ t;
  int previous_time_recording;

  ctype_ brood_resources;

  Simulation(const params& par,
             ctype_ (*share_func)(individual*, individual*, ctype_, size_t)) :
             p(par),
             rndgen(p.mean_threshold, p.sd_threshold) {
  
    colony = std::vector< individual >(p.colony_size);

    t = 0.0;
    previous_time_recording = -1;
    brood_resources = 0.0;
    for (auto& i : colony) {
      i.initialize(p, rndgen, share_func);
    }
  }

  void update_colony() {

    nurses.clear();
    auto focal_individual = colony.begin();
    ctype_ new_t = std::numeric_limits<ctype_>::max();

    for (auto i = colony.begin(); i != colony.end(); ++i) {
      if (i->get_task() == task::nurse) {
        nurses.push_back(&(*i));
      }
      if (i->get_next_t() < new_t) {
        new_t = i->get_next_t();
        focal_individual = i;
      }
    }

    assert(new_t >= t);
    t = new_t;

    focal_individual->update(t, p, rndgen, nurses);
  }

  void run_simulation() {

    while(t < p.simulation_time) {
      update_colony();
    }
    // end roll call:
    for (auto& i : colony) {
      i.update_tasks(t);
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
     size_t cnt = 0;
     for (const auto& i : colony) {
      out_file << t << "\t"
                << cnt << "\t"
                << static_cast<int>(i.get_task()) << "\t"
                << i.get_fat_body() << "\t"
                << i.get_crop() << "\n";
      cnt++;
     }
     out_file.close();
   }
};

std::unique_ptr<Simulation> create_simulation(const params& p) {
  switch(p.model_type) {
    case 0: {
      return std::make_unique<Simulation>(p, no_sharing);
      break;
    }
    case 1: {
      return std::make_unique<Simulation>(p, fair_sharing);
      break;
    }
    case 2: {
      return std::make_unique<Simulation>(p, dominance_sharing);
      break;
    }
    case 3: {
      return std::make_unique<Simulation>(p, fatbody_sharing);
      break;
    }
  }
  // default
  return std::make_unique<Simulation>(p, no_sharing);
}


#endif /* simulation_h */
