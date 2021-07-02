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

  std::string dol_file_name = "dol_default.txt";
  std::string output_file_name = "output_default.txt";

  int simulation_time = 10000; // length of simulation
  int data_interval = 1; // data is written simulationTime / dataInterval times
  int colony_size = 100; // number of individuals per colony

  int model_type = 3; // 0 = no sharing, 1 = random sharing, 2 = dominance sharing, 3 = evolving dominance

  int max_number_interactions = 3; // max number of interactions with nurses at foraging return
};

struct ind_param {
  float metabolic_cost_nurses = 0.5;
  float metabolic_cost_foragers = 0.5;
  float max_fat_body = 12.0; // new individuals have a fat body that has this size at maximum
  float init_fat_body = 10.0;
  float crop_size = 5.0; // maximal resources that can be carried in crop
  float proportion_fat_body_forager = 0.2; // proportion of resources allocated to the fatbody by foragers
  float proportion_fat_body_nurse   = 0.2;  // proportion of resources allocated to the fatbody by nurses
  float mean_dominance = 5.0;
  float sd_dominance = 2.0;
  float mean_threshold = 5.0;
  float sd_threshold = 1.7;
  float food_handling_time = 0.5;
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
