//
//  main.cpp
//  dol_fatbody_tj
//
//  Created by Thijs Janzen on 06/04/2021.
//  Copyright © 2021 Thijs Janzen. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "parameters.h"
#include "simulation.h"
#include "individual.h"
#include "statistics.h"
#include <chrono>

int main(int argc, char* argv[]) {
  try {

    std::string file_name = (argc > 2) ? argv[1] : "config.ini";

    std::cout << "reading from config file: " << file_name << "\n";
    std::ifstream test_file(file_name.c_str());
    if (!test_file.is_open()) {
      throw std::runtime_error("can't find config file");
    }
    test_file.close();

    params sim_par_in(file_name);

    output::write_dol_headers(sim_par_in.param_names_to_record,
                              sim_par_in.dol_file_name,
                              sim_par_in.window_file_name,
                              sim_par_in.data_interval);

    for (size_t num_repl = 0; num_repl < sim_par_in.num_replicates; ++num_repl) {
        std::unique_ptr<Simulation> sim = create_simulation(sim_par_in);

        auto clock_start = std::chrono::system_clock::now();
        sim->run();


        if (sim_par_in.data_interval == 0) {
          output::write_ants_to_file(sim->colony,
                                     sim_par_in.output_file_name, num_repl);
          
          output::write_dol_sliding_window(sim->colony,
                                           sim_par_in.window_size,
                                           sim_par_in.window_step_size,
                                           static_cast<float>(sim_par_in.simulation_time),
                                           sim_par_in.window_file_name,
                                           num_repl);
        }

        output::write_dol_to_file(sim->colony,
                                  sim_par_in.params_to_record,
                                  sim_par_in.dol_file_name,
                                  num_repl,
                                  sim_par_in.burnin,
                                  static_cast<float>(sim_par_in.simulation_time));

      auto clock_now = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_seconds = clock_now - clock_start;
      std::cout << "this took: " << elapsed_seconds.count() << "seconds\n";
    }
    
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
