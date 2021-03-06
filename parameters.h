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

using ctype_ = float;

enum share_model {no, fair, dominance, fat_body, max_model};

struct params {

  params() {};

  params(const std::string& file_name) {
    read_parameters_from_ini(file_name);
  }

  std::string dol_file_name = "dol_default.txt";
  std::string output_file_name = "output_default.txt";
  std::string window_file_name = "window_default.txt";

  size_t simulation_time = 10000; // length of simulation
  int data_interval = 1; // data is written simulationTime / dataInterval times
  size_t colony_size = 100; // number of individuals per colony

  share_model model_type = share_model::fat_body; // 0 = no sharing, 1 = random sharing, 2 = dominance sharing, 3 = changing dominance

  size_t max_number_interactions = 3; // max number of interactions with nurses at foraging return

  ctype_ metabolic_cost_nurses = 0.5f;
  ctype_ metabolic_cost_foragers = 0.5f;
  ctype_ max_fat_body = 12.0f; // new individuals have a fat body that has this size at maximum
  ctype_ init_fat_body = 10.0f;
  ctype_ mean_threshold = 5.0f;
  ctype_ sd_threshold = 1.7f;
  ctype_ food_handling_time = 0.5f;

  ctype_ resource_amount = 1.0f;
  ctype_ foraging_time = 5.0f;

  size_t num_replicates = 10;

  ctype_ burnin = 0.1f;
  ctype_ window_size = 100.f; // used for sliding window recording of DoL stats. Only used when data_interval = 0.
  ctype_ window_step_size =  1.f;

  ctype_ soft_max = 1.0;

  std::string temp_params_to_record;
  std::vector < std::string > param_names_to_record;
  std::vector < ctype_ > params_to_record;

  void read_parameters_from_ini(const std::string& file_name) {
    ConfigFile from_config(file_name);

    dol_file_name                 = from_config.getValueOfKey<std::string>("dol_file_name");
    output_file_name              = from_config.getValueOfKey<std::string>("output_file_name");
    window_file_name              = from_config.getValueOfKey<std::string>("window_file_name");
    simulation_time               = from_config.getValueOfKey<size_t>("simulation_time");
    data_interval                 = from_config.getValueOfKey<int>("data_interval");
    colony_size                   = from_config.getValueOfKey<size_t>("colony_size");
    model_type                    = static_cast<share_model>(from_config.getValueOfKey<size_t>("model_type"));
    max_number_interactions       = from_config.getValueOfKey<size_t>("max_number_interactions");
    metabolic_cost_nurses         = from_config.getValueOfKey<ctype_>("metabolic_cost_nurses");
    metabolic_cost_foragers       = from_config.getValueOfKey<ctype_>("metabolic_cost_foragers");
    max_fat_body                  = from_config.getValueOfKey<ctype_>("max_fat_body");
    init_fat_body                 = from_config.getValueOfKey<ctype_>("init_fat_body");
    mean_threshold                = from_config.getValueOfKey<ctype_>("mean_threshold");
    sd_threshold                  = from_config.getValueOfKey<ctype_>("sd_threshold");
    food_handling_time            = from_config.getValueOfKey<ctype_>("food_handling_time");
    resource_amount               = from_config.getValueOfKey<ctype_>("resource_amount");
    foraging_time                 = from_config.getValueOfKey<ctype_>("foraging_time");
    num_replicates                = from_config.getValueOfKey<size_t>("num_replicates");
    temp_params_to_record         = from_config.getValueOfKey<std::string>("params_to_record");
    param_names_to_record         = split(temp_params_to_record);
    params_to_record              = create_params_to_record(param_names_to_record);
    burnin                        = from_config.getValueOfKey<ctype_>("burnin");
    window_size                   = from_config.getValueOfKey<ctype_>("window_size");
    window_step_size              = from_config.getValueOfKey<ctype_>("window_step_size");
    soft_max                      = from_config.getValueOfKey<ctype_>("soft_max");
  }

  std::vector< std::string > split(std::string s) {
    // code from: https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
    std::vector< std::string > output;
    std::string delimiter = ",";
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
      token = s.substr(0, pos);
      output.push_back(token);
      s.erase(0, pos + delimiter.length());
    }
    output.push_back(s); // only last entry now
    return output;
  }

  std::vector< ctype_ > create_params_to_record(const std::vector< std::string >& param_names) {
    std::vector< ctype_ > output;
    for (auto i : param_names) {
      output.push_back(get_val(i));
    }
    return output;
  }

  ctype_ get_val(std::string s) {
    if (s == "simulation_time")             return static_cast<ctype_>(simulation_time);
    if (s == "data_interval")               return static_cast<ctype_>(data_interval);
    if (s == "colony_size")                 return static_cast<ctype_>(colony_size);
    if (s == "model_type")                  return static_cast<ctype_>(model_type);
    if (s == "max_number_interactions")     return static_cast<ctype_>(max_number_interactions);
    if (s == "metabolic_cost_nurses")       return metabolic_cost_nurses;
    if (s == "metabolic_cost_foragers")     return metabolic_cost_foragers;
    if (s == "max_fat_body")                return max_fat_body;
    if (s == "init_fat_body")               return init_fat_body;
    if (s == "mean_threshold")              return mean_threshold;
    if (s == "sd_threshold")                return sd_threshold;
    if (s == "food_handling_time")          return food_handling_time;
    if (s == "resource_amount")             return resource_amount;
    if (s == "foraging_time")               return foraging_time;
    if (s == "burnin")                      return burnin;
    if (s == "num_replicates")              return static_cast<ctype_>(num_replicates);

    throw std::runtime_error("can not find parameter");
    return -1.f; // FAIL
  }

};

#endif /* parameters_h */
