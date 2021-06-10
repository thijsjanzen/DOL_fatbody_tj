create_json <- function(json_file_name = "test.json",
                        resource_amount = 5.0,
                        foraging_time = 5.0,
                        metabolic_cost_nurses = 0.2,
                        metabolic_cost_foragers = 0.2,
                        min_fat_body = 8.0,
                        max_fat_body = 12.0,
                        min_for_abi = 1.0,
                        max_for_abi = 1.0,
                        crop_size = 5.0,
                        fat_body_size = 15.0,
                        min_share = 0.0,
                        max_share = 1.0,
                        proportion_fat_body_forager = 0.2,
                        proportion_fat_body_nurse   = 0.2,
                        simulation_time = 10000,
                        data_interval = 100.0,
                        colony_size = 100,

                        dt = 0.01,
                        half_point = 5,
                        mean_dominance = 5,
                        sd_dominance = 2,

                        max_number_interactions = 3,

                        model_type = 3,       # 0 = no sharing, 1 = random sharing,
                        # 2 = dominance sharing, 3 = evolving dominance

                        dol_file_name = "dol.txt",
                        output_file_name = "output.txt") {


  env_param <- data.frame(
    resource_amount = resource_amount,
    foraging_time = foraging_time);

  ind_param <- data.frame(
    metabolic_cost_nurses = metabolic_cost_nurses,
    metabolic_cost_foragers = metabolic_cost_foragers,
    min_fat_body = min_fat_body,
    max_fat_body = max_fat_body,
    min_for_abi = min_for_abi,
    max_for_abi = max_for_abi,
    crop_size = crop_size,
    fat_body_size = fat_body_size,
    min_share = min_share,
    max_share = max_share,
    proportion_fat_body_forager = proportion_fat_body_forager,
    proportion_fat_body_nurse   = proportion_fat_body_nurse,
    mean_dominance = mean_dominance,
    sd_dominance = sd_dominance,
    half_point = half_point
  )

  meta_param <- data.frame(
    simulation_time = simulation_time,
    data_interval = data_interval,
    colony_size = colony_size,

    max_number_interactions = max_number_interactions,

    model_type = model_type,       # 0 = no sharing, 1 = random sharing,
    # 2 = dominance sharing, 3 = evolving dominance

    dol_file_name = dol_file_name,
    output_file_name = output_file_name,
    dt = dt
  )

  sim_param <- list("env_param" = env_param,
                    "ind_param" = ind_param,
                    "meta_param" = meta_param)

  output <- list("sim_param" = sim_param)

  x <- rjson::toJSON(output, indent = 2)
  write(x, file = json_file_name)
}
