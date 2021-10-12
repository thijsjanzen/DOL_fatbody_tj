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
#include <chrono>

int main(int argc, char* argv[]) {
  try {
    if (argc < 1) { // needs run name
             std::cerr << "Usage: " << argv[0] << "parameter name" << std::endl;
             return 1;
    }

    std::string file_name = argv[1];

    std::cout << "reading from config file: " << file_name << "\n";
    std::ifstream test_file(file_name.c_str());
    if (!test_file.is_open()) {
      std::cerr << "ERROR! could not read ini file\n";
      std::cerr << "Did you provide a file name as command line argument?";
      return 1;
    }
    test_file.close();

    params sim_par_in(file_name);

    // create fake simulation object to write dol header.
    // This should be streamlined better.
    // TODO: move writing functions outside simulation object.
    Simulation sim_fake(sim_par_in);
    sim_fake.write_dol_header(sim_par_in.param_names_to_record,
                              sim_par_in.dol_file_name);

    for (size_t num_repl = 0; num_repl < sim_par_in.num_replicates; ++num_repl) {
        Simulation sim(sim_par_in);

        auto clock_start = std::chrono::system_clock::now();
        sim.run_simulation();
        auto clock_now = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = clock_now - clock_start;
        std::cout << "this took: " << elapsed_seconds.count() << "seconds\n";

        if (sim_par_in.data_interval == 0) {
          sim.write_ants_to_file(sim_par_in.output_file_name, num_repl);
        }

        sim.write_dol_to_file(sim_par_in.param_names_to_record,
                              sim_par_in.params_to_record,
                              sim_par_in.dol_file_name,
                              num_repl);
    }
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
