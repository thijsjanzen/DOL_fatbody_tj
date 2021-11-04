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
    ctype_ new_t = focal_individual->get_next_t();

   /*
    ctype_ new_t = std::numeric_limits<ctype_>::max();

    for (auto i = colony.begin(); i != colony.end(); ++i) {
      if (i->get_task() == task::nurse) {
        nurses.push_back(&(*i));
      }
      
      if (i->get_next_t() < new_t) {
        new_t = i->get_next_t();
        focal_individual = i;
      }
    }*/

    assert(new_t >= t);
    t = new_t;

    focal_individual->update(t, p, rndgen, nurses);
  }

  void run() {
    while(t < p.simulation_time) {
      update_colony();
    }
    // end roll call, for data purposes:
    for (auto& i : colony) {
      i.update_data(t);
    }
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
