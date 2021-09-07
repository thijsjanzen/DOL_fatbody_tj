//
//  main.cpp
//  dol_fatbody_tj
//
//  Created by Thijs Janzen on 06/04/2021.
//  Copyright © 2021 Thijs Janzen. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "simulation.h"
#include "individual.h"
#include "parameters.h"
#include "json.hpp"
#include <chrono>

int main(int argc, char* argv[]) {
  try {
    if (argc < 1) { // needs run name
             std::cerr << "Usage: " << argv[0] << "parameter name" << std::endl;
             return 1;
    }

    auto file_name = argv[1];

    std::cout << "reading from JSON file: " << file_name << "\n";

    nlohmann::json json_in;
    std::ifstream is(file_name);

    if(!is.is_open()) {
      std::cerr << "Can not open JSON file\n";
      return 1;
    }

    is >> json_in;
    sim_param sim_par_in = json_in.get<sim_param>();

    std::cout << "JSON file read\n";

    Simulation sim(sim_par_in);


    auto clock_start = std::chrono::system_clock::now();
    sim.run_simulation();
    auto clock_now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = clock_now - clock_start;
    std::cout << "this took: " << elapsed_seconds.count() << "seconds\n";

    if (sim_par_in.get_meta_param().data_interval == 0) {
      sim.write_ants_to_file(sim_par_in.get_meta_param().output_file_name);
    }

    // the vector params_of_interest can be modified to contain anything you are varying
    // it is then subsequently added to the output file in front of the DoL measures.
    std::vector< double > params_of_interest = {
      sim_par_in.get_ind_param().food_handling_time,
sim_par_in.get_ind_param().metabolic_cost_nurses,
sim_par_in.get_ind_param().metabolic_cost_foragers,
sim_par_in.get_ind_param().init_fat_body,
sim_par_in.get_ind_param().max_fat_body,      
sim_par_in.get_ind_param().crop_size,
sim_par_in.get_env_param().resource_amount,
sim_par_in.get_env_param().foraging_time
};

    sim.write_dol_to_file(params_of_interest,
                         sim_par_in.get_meta_param().dol_file_name);
    
    return 0;
  }
  catch (const std::exception& err) {
    std::cerr << err.what() << '\n';
  }
  catch (const char* err) {
    std::cerr << err << '\n';
  }
  catch (...) {
    std::cerr << "unknown exception\n";
  }
  std::cerr << "bailing out.\n";

  return -1;
}
