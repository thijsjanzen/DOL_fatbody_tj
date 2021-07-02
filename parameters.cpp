#include "json.hpp"
#include "parameters.h"


// define to_json / from_json pair for env_param
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(env_param,
  resource_amount,
  foraging_time
)


void to_json(nlohmann::json& j, const meta_param& t)
{
  NLOHMANN_JSON_TO(simulation_time);
  NLOHMANN_JSON_TO(data_interval);
  NLOHMANN_JSON_TO(colony_size);
  NLOHMANN_JSON_TO(max_number_interactions);
  NLOHMANN_JSON_TO(dol_file_name);
  NLOHMANN_JSON_TO(output_file_name);
}

void from_json(const nlohmann::json& j, meta_param& t)
{
  NLOHMANN_JSON_FROM(simulation_time);
  NLOHMANN_JSON_FROM(data_interval);
  NLOHMANN_JSON_FROM(colony_size);
  NLOHMANN_JSON_FROM(max_number_interactions);
  NLOHMANN_JSON_FROM(dol_file_name);
  NLOHMANN_JSON_FROM(output_file_name);
}


// define to_json / from_json pair for ind_param
// we can't use the fancy NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE macro
// because we have too many members :(
// However NLOHMANN_JSON_TO/FROM is nearly as convenient
void to_json(nlohmann::json& j, const ind_param& t)
{
  NLOHMANN_JSON_TO(metabolic_cost_nurses);
  NLOHMANN_JSON_TO(metabolic_cost_foragers);
  NLOHMANN_JSON_TO(max_fat_body);
  NLOHMANN_JSON_TO(crop_size);
  NLOHMANN_JSON_TO(init_fat_body);
  NLOHMANN_JSON_TO(proportion_fat_body_forager);
  NLOHMANN_JSON_TO(proportion_fat_body_nurse);
  NLOHMANN_JSON_TO(mean_dominance);
  NLOHMANN_JSON_TO(sd_dominance);
  NLOHMANN_JSON_TO(mean_threshold);
  NLOHMANN_JSON_TO(sd_threshold);
  NLOHMANN_JSON_TO(food_handling_time);
}

void from_json(const nlohmann::json& j, ind_param& t)
{
  NLOHMANN_JSON_FROM(metabolic_cost_nurses);
  NLOHMANN_JSON_FROM(metabolic_cost_foragers);
  NLOHMANN_JSON_FROM(max_fat_body);
  NLOHMANN_JSON_FROM(crop_size);
  NLOHMANN_JSON_FROM(init_fat_body);
  NLOHMANN_JSON_FROM(proportion_fat_body_forager);
  NLOHMANN_JSON_FROM(proportion_fat_body_nurse);
  NLOHMANN_JSON_FROM(mean_dominance);
  NLOHMANN_JSON_FROM(sd_dominance);
  NLOHMANN_JSON_FROM(mean_threshold);
  NLOHMANN_JSON_FROM(sd_threshold);
  NLOHMANN_JSON_FROM(food_handling_time);
}


// define to_json / from_json pair for sim_param
// members are private here - we use accessors
void to_json(nlohmann::json& j, const sim_param& t)
{
  j["sim_param"] = {
    {"env_param", t.get_env_param()},
    {"ind_param", t.get_ind_param()},
    {"meta_param", t.get_meta_param()}
  };
}


// at least we have an public constructor...
void from_json(const nlohmann::json& j, sim_param& t)
{
  auto jsp = j.at("sim_param");
  auto ep = jsp.at("env_param").get<env_param>();
  auto ip = jsp.at("ind_param").get<ind_param>();
  auto mp = jsp.at("meta_param").get<meta_param>();
  t = sim_param(ep, ip, mp);
}
