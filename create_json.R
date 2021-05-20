env_param <- data.frame(
                        resource_amount = 5.0,
                        foraging_time = 5.0);

ind_param <- data.frame(
  metabolic_cost_nurses = 1.0,
  metabolic_cost_foragers = 1.0,
  min_fat_body = 8.0,
  max_fat_body = 12.0,
  min_for_abi = 1.0,
  max_for_abi = 1.0,
  crop_size = 5.0,
  fat_body_size = 15.0,
  min_share = 0.0,
  max_share = 1.0,
  proportion_fat_body_forager = 0.2,
  proportion_fat_body_nurse   = 0.2
)

meta_param <- data.frame(
  simulation_time = 10000,
  data_interval = 1.0,
  colony_size = 100,

  dominance_interaction = TRUE,

  threshold_mean = 5.0,
  threshold_sd = 2.0,

  max_number_interactions = 3

  model_type = 0 # 0 = no sharing, 1 = random sharing, 2 = dominance sharing, 3 = evolving dominance
)

sim_param <- list("env_param" = env_param,
                  "ind_param" = ind_param,
                  "meta_param" = meta_param)

output <- list("sim_param" = sim_param)


x <- rjson::toJSON(output, indent = 2)
write(x, file = "test.json")