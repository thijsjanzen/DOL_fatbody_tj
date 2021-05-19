//
//  parameters.h
//  json_parameters_dol
//
//  Created by Thijs Janzen on 30/03/2021.
//  Copyright © 2021 Thijs Janzen. All rights reserved.
//

#ifndef parameters_h
#define parameters_h

#include "json.hpp"

struct meta_param {
  int simulation_time = 10000; // length of simulation
  int data_interval = 1; // data is written simulationTime / dataInterval times
  int colony_size = 100; // number of individuals per colony

  bool dominance_interaction = 1;

  int model_type = 3; // 0 = no sharing, 1 = random sharing, 2 = dominance sharing, 3 = evolving dominance

  float threshold_mean = 5.0;
  float threshold_sd = 2.0;

  int max_number_interactions = 3; // max number of interactions with nurses at foraging return
};

struct ind_param {
  float metabolic_cost_nurses = 0.5;
  float metabolic_cost_foragers = 0.5;
  double min_fat_body = 8.0; // new individuals have a fat body that has at least this size
  double max_fat_body = 12.0; // new individuals have a fat body that has this size at maximum
  double min_for_abi = 1.0; // minimal foraging ability for new individuals
  double max_for_abi = 1.0;  // maximal foraging ability for new individuals
  float crop_size = 5.0; // maximal resources that can be carried in crop
  double fat_body_size = 15.0; // maximal resources that can be accumulated in fatbody
  double min_share = 0.0;
  double max_share = 1.0;
  float proportion_fat_body_forager = 0.2; // proportion of resources allocated to the fatbody by foragers
  float proportion_fat_body_nurse   = 0.2;  // proportion of resources allocated to the fatbody by nurses
};

struct env_param {
  float resource_amount = 1.0;
  float foraging_time = 5.0;
};


class sim_param
{
public:
    sim_param() {};
    sim_param(env_param e, ind_param i, meta_param m) :
      m_env_param(e), m_ind_param(i), m_meta_param(m)
    {}

    ///Gets const reference to population parameter
    const env_param& get_env_param() const noexcept {return m_env_param;}

    ///Gets const reference to metaparameters
    const meta_param& get_meta_param() const noexcept {return  m_meta_param;}

    ///Gets const reference to ind parameter
    const ind_param& get_ind_param() const noexcept {return m_ind_param;}

private:
    ///Parameters for the environment
    env_param m_env_param;

    ///Parameters pertaining individuals
    ind_param m_ind_param;

    ///Parameters concerning the simulation, such as duration, number of replicates etc
    meta_param m_meta_param;
};

// declare to_json / from_json pair for sim_param
void to_json(nlohmann::json& j, const sim_param& p);
void from_json(const nlohmann::json& j, sim_param& p);

#endif /* parameters_h */
