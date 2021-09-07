//
//  parameters.h
//  json_parameters_dol
//
//  Created by Thijs Janzen on 30/03/2021.
//  Copyright © 2021 Thijs Janzen. All rights reserved.
//

#ifndef parameters_h
#define parameters_h

// #include <cmath>
#include "config_parser.h"
#include <string>
#include <vector>


struct params {

  params(const std::string& file_name) {
    read_parameters_from_ini(file_name);
  }

  std::string dol_file_name = "dol_default.txt";
  std::string output_file_name = "output_default.txt";

  size_t simulation_time = 10000; // length of simulation
  int data_interval = 1; // data is written simulationTime / dataInterval times
  size_t colony_size = 100; // number of individuals per colony

  size_t model_type = 3; // 0 = no sharing, 1 = random sharing, 2 = dominance sharing, 3 = evolving dominance

  size_t max_number_interactions = 3; // max number of interactions with nurses at foraging return

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

  float resource_amount = 1.0;
  float foraging_time = 5.0;

  std::string temp_params_to_record;
  std::vector < std::string > param_names_to_record;
  std::vector < float > params_to_record;

  void read_parameters_from_ini(const std::string& file_name) {
    ConfigFile from_config(file_name);

    dol_file_name                 = from_config.getValueOfKey<std::string>("dol_file_name");
    output_file_name              = from_config.getValueOfKey<std::string>("output_file_name");
    simulation_time               = from_config.getValueOfKey<size_t>("simulation_time");
    data_interval                 = from_config.getValueOfKey<int>("data_interval");
    colony_size                   = from_config.getValueOfKey<size_t>("colony_size");
    model_type                    = from_config.getValueOfKey<size_t>("model_type");
    max_number_interactions       = from_config.getValueOfKey<size_t>("max_number_interactions");
    metabolic_cost_nurses         = from_config.getValueOfKey<float>("metabolic_cost_nurses");
    metabolic_cost_foragers       = from_config.getValueOfKey<float>("metabolic_cost_foragers");
    max_fat_body                  = from_config.getValueOfKey<float>("max_fat_body");
    init_fat_body                 = from_config.getValueOfKey<float>("init_fat_body");
    crop_size                     = from_config.getValueOfKey<float>("crop_size");
    proportion_fat_body_forager   = from_config.getValueOfKey<float>("proportion_fat_body_forager");
    proportion_fat_body_nurse     = from_config.getValueOfKey<float>("proportion_fat_body_nurse");
    mean_dominance                = from_config.getValueOfKey<float>("mean_dominance");
    sd_dominance                  = from_config.getValueOfKey<float>("sd_dominance");
    mean_threshold                = from_config.getValueOfKey<float>("mean_threshold");
    sd_threshold                  = from_config.getValueOfKey<float>("sd_threshold");
    food_handling_time            = from_config.getValueOfKey<float>("food_handling_time");
    resource_amount               = from_config.getValueOfKey<float>("resource_amount");
    foraging_time                 = from_config.getValueOfKey<float>("foraging_time");
    temp_params_to_record         = from_config.getValueOfKey<std::string>("params_to_record");
    param_names_to_record         = split(temp_params_to_record);
    params_to_record              = create_params_to_record(param_names_to_record);
  }

  std::vector< std::string > split(std::string s) {
    std::vector< std::string > output;
    std::string delimiter = ",";
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
      token = s.substr(0, pos);
      output.push_back(token);
      s.erase(0, pos + delimiter.length());
    }
    return output;
  }

  std::vector< float > create_params_to_record(const std::vector< std::string >& param_names) {
    std::vector< float > output;
    for (auto i : param_names) {
      output.push_back(get_val(i));
    }
    return output;
  }

  float get_val(std::string s) {
    if (s == "simulation_time")             return static_cast<float>(simulation_time);
    if (s == "data_interval")               return static_cast<float>(data_interval);
    if (s == "colony_size")                 return static_cast<float>(colony_size);
    if (s == "model_type")                  return static_cast<float>(model_type);
    if (s == "max_number_interactions")     return static_cast<float>(max_number_interactions);
    if (s == "metabolic_cost_nurses")       return metabolic_cost_nurses;
    if (s == "metabolic_cost_foragers")     return metabolic_cost_foragers;
    if (s == "max_fat_body")                return max_fat_body;
    if (s == "init_fat_body")               return init_fat_body;
    if (s == "crop_size")                   return crop_size;
    if (s == "proportion_fat_body_forager") return proportion_fat_body_forager;
    if (s == "proportion_fat_body_nurse")   return proportion_fat_body_nurse;
    if (s == "mean_dominance")              return mean_dominance;
    if (s == "sd_dominance")                return sd_dominance;
    if (s == "mean_threshold")              return mean_threshold;
    if (s == "sd_threshold")                return sd_threshold;
    if (s == "food_handling_time")          return food_handling_time;
    if (s == "resource_amount")             return resource_amount;
    if (s == "foraging_time")               return foraging_time;

    throw std::runtime_error("can not find parameter");
    return -1.f; // FAIL
  }

};





#endif /* parameters_h */
