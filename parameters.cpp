#include "json.hpp"
#include "parameters.h"


// define to_json / from_json pair for env_param
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(env_param,
  resource_amount,
  foraging_time
)


// define to_json / from_json pair for meta_param
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(meta_param,
  simulation_time,
  data_interval,
  colony_size,
  dominance_interaction,
  threshold_mean,
  threshold_sd,
  max_number_interactions
)

// define to_json / from_json pair for ind_param
// we can't use the fancy NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE macro
// because we have too many members :(
// However NLOHMANN_JSON_TO/FROM is nearly as convenient
void to_json(nlohmann::json& j, const ind_param& t)
{
  NLOHMANN_JSON_TO(metabolic_cost_nurses);
  NLOHMANN_JSON_TO(metabolic_cost_foragers);
  NLOHMANN_JSON_TO(min_fat_body);
  NLOHMANN_JSON_TO(max_fat_body);
  NLOHMANN_JSON_TO(min_for_abi);
  NLOHMANN_JSON_TO(max_for_abi);
  NLOHMANN_JSON_TO(crop_size);
  NLOHMANN_JSON_TO(fat_body_size);
  NLOHMANN_JSON_TO(min_share);
  NLOHMANN_JSON_TO(max_share);
  NLOHMANN_JSON_TO(proportion_fat_body_forager);
  NLOHMANN_JSON_TO(proportion_fat_body_nurse);
}

void from_json(const nlohmann::json& j, ind_param& t)
{
  NLOHMANN_JSON_FROM(metabolic_cost_nurses);
  NLOHMANN_JSON_FROM(metabolic_cost_foragers);
  NLOHMANN_JSON_FROM(min_fat_body);
  NLOHMANN_JSON_FROM(max_fat_body);
  NLOHMANN_JSON_FROM(min_for_abi);
  NLOHMANN_JSON_FROM(max_for_abi);
  NLOHMANN_JSON_FROM(crop_size);
  NLOHMANN_JSON_FROM(fat_body_size);
  NLOHMANN_JSON_FROM(min_share);
  NLOHMANN_JSON_FROM(max_share);
  NLOHMANN_JSON_FROM(proportion_fat_body_forager);
  NLOHMANN_JSON_FROM(proportion_fat_body_nurse);
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
