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

  Simulation(const params& par,
             std::vector<ctype_> (*share_func_grouped)(individual*, std::vector<individual*>, ctype_, size_t)) :
             p(par),
             rndgen(p.mean_threshold, p.sd_threshold) {
  
     colony = std::vector< individual >(p.colony_size);
     for (auto& i : colony) {
       i.initialize(p, rndgen, share_func_grouped);
     }
     t = 0.0;
     previous_time_recording = -1;
  }

  void update_colony() {

    nurses.clear();

    auto focal_individual = colony.begin();

    for (auto i = colony.begin(); i != colony.end(); ++i) {
      if (i->get_next_t() < focal_individual->get_next_t()) {
         focal_individual = i;
      }
    }

    if (focal_individual->get_task() == task::forage) {
        for (auto i = colony.begin(); i != colony.end(); ++i) {
          if (i->get_task() == task::nurse) {
            nurses.push_back(&(*i));
          }
        }
    }
    t = focal_individual->get_next_t();
    if (t > p.simulation_time) return;

    focal_individual->update(t, p, rndgen, nurses);
  }

  void run() {
    while(t < p.simulation_time) {
      update_colony();
    }
    t = p.simulation_time;
    // end roll call, for data purposes:
    for (auto& i : colony) {
      i.update_fatbody(t);
      i.update_data(t);
    }
  }
};

std::unique_ptr<Simulation> create_simulation(const params& p) {

  switch(p.model_type) {
    case share_model::no : {
      return std::make_unique<Simulation>(p, no_sharing_grouped);
      break;
    }
    case share_model::fair: {
      return std::make_unique<Simulation>(p, fair_sharing_grouped);
      break;
    }
    case share_model::dominance: {
      return std::make_unique<Simulation>(p, dominance_sharing_grouped);
      break;
    }
    case share_model::fat_body: {
      return std::make_unique<Simulation>(p, fatbody_sharing_grouped);
      break;
    }
    case share_model::max_model: {
      throw std::exception();
      break;
    }

    default: {
      throw std::exception();
        break;
    }

  }
}


#endif /* simulation_h */
