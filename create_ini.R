create_config <- function(config_file_name = "config.ini",
                          resource_amount = 5.0,
                          foraging_time = 5.0,
                          metabolic_cost_nurses = 0.2,
                          metabolic_cost_foragers = 0.2,
                          max_fat_body = 12.0,
                          crop_size = 5.0,
                          init_fat_body = 10.0,
                          proportion_fat_body_forager = 0.2,
                          proportion_fat_body_nurse   = 0.2,
                          simulation_time = 10000,
                          data_interval = 100.0,
                          colony_size = 100,
                          mean_dominance = 5,
                          sd_dominance = 2,
                          mean_threshold = 5,
                          sd_threshold = 1.7,
                          food_handling_time = 0.1,
                          max_number_interactions = 3,
                          model_type = 3,
                          num_replicates = 10,
                          dol_file_name = "dol.txt",
                          output_file_name = "output.txt",
                          window_file_name = "window.txt",
                          forager_sharing_at_default = 0.0,
                          burnin = 0.1,
                          window_size = window_size,
                          window_step_size = window_step_size,
                          params_to_record = "food_handling_time,metabolic_cost_nurses,metabolic_cost_foragers,init_fat_body,max_fat_body,crop_size,resource_amount,foraging_time") {

  newini <- list()
  newini[["env_param"]] <-
    list("resource_amount" = resource_amount,
         "foraging_time" = foraging_time)

  newini[["ind_param"]] <-
    list("metabolic_cost_nurses" = metabolic_cost_nurses,
         "metabolic_cost_foragers" = metabolic_cost_foragers,
         "max_fat_body" = max_fat_body,
         "crop_size" = crop_size,
         "init_fat_body" = init_fat_body,
         "proportion_fat_body_forager" = proportion_fat_body_forager,
         "proportion_fat_body_nurse"   = proportion_fat_body_nurse,
         "mean_dominance" = mean_dominance,
         "sd_dominance" = sd_dominance,
         "mean_threshold" = mean_threshold,
         "sd_threshold" = sd_threshold,
         "food_handling_time" = food_handling_time,
         "forager_sharing_at_default" = forager_sharing_at_default)

  newini[["meta_param"]] <-
    list("simulation_time" = simulation_time,
         "data_interval" = data_interval,
         "colony_size" = colony_size,
         "num_replicates" = num_replicates,
         "max_number_interactions" = max_number_interactions,
         "burnin" = burnin,
         "model_type" = model_type,       # 0 = no sharing, 1 = random sharing,
         # 2 = dominance sharing, 3 = evolving dominance
         "window_size" = window_size,
         "window_step_size" = window_step_size,
         "dol_file_name" = dol_file_name,
         "output_file_name" = output_file_name,
         "window_file_name"= window_file_name,
         "params_to_record" = params_to_record)

  ini::write.ini(newini, config_file_name)
}

create_config(config_file_name = "test.ini")


