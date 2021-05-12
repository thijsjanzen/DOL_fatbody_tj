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

 /* sim_param sim_par_in2; // use default values;
  nlohmann::json json_out;
  json_out = sim_par_in2;
  std::ofstream os("test2.json");
  os << json_out;
  os.close();

  nlohmann::json json_in;
  std::ifstream is("test.json");
  is >> json_in;
  sim_param sim_par_in = json_in.get<sim_param>();
*/
  try {
        sim_param sim_par_in; // use default values;
     //   nlohmann::json json_out;
     //   json_out = sim_par_in;
     //   std::ofstream os("test.json");
     //   os << json_out;

        Simulation sim(sim_par_in.get_meta_param().colony_size,
                       sim_par_in);

        sim.run_simulation();

        sim.write_ants_to_file("test.txt");
        sim.write_dol_to_file("dol.txt");

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
