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

int main() {
  try {
    nlohmann::json json_in;
    std::ifstream is("test.json");
    is >> json_in;
    sim_param sim_par_in = json_in.get<sim_param>();
    Simulation sim(sim_par_in);

    sim.run_simulation();

    sim.write_ants_to_file(sim_par_in.get_meta_param().output_file_name);
    sim.write_dol_to_file(sim_par_in.get_meta_param().dol_file_name);

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
