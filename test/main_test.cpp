#define CATCH_CONFIG_MAIN
#include "catch.h"

#include "../parameters.h"
#include "../simulation.h"
#include "../individual.h"

#include <fstream>
#include <string>


TEST_CASE("TEST parameters") {

  // we test here the ability to extract parameters to record
  params parameters;
  auto s = "resource_amount,foraging_time";
  auto vec = parameters.split(s);
  REQUIRE(vec.size() == 2);
  CHECK(vec[0] == "resource_amount");
  CHECK(vec[1] == "foraging_time");

  auto vals = parameters.create_params_to_record(vec);
  CHECK(vals[0] == parameters.resource_amount);
  CHECK(vals[1] == parameters.foraging_time);
}

TEST_CASE("TEST simulation") {

  params parameters;
  parameters.model_type = 1;
  rnd_t rndgen(42);

  Simulation test_sim(parameters);

  CHECK(test_sim.colony.size() == parameters.colony_size);

  bool checked = test_sim.check_time_interval(1.f, 2.f, 1.f);
  CHECK(checked == true);
  checked = test_sim.check_time_interval(1.f, 2.f, 2.f);
  CHECK(checked == false);


  test_sim.colony[0].set_previous_task();
  test_sim.colony[0].set_current_task(forage);
  test_sim.update_nurse_list(&test_sim.colony[0]);
  // should be removed from nurses now.
  test_sim.nurses.size() == parameters.colony_size - 1;


  test_sim.colony[10].set_current_task(forage);
  test_sim.remove_from_nurses(10);
  REQUIRE(test_sim.nurses.size() == parameters.colony_size - 2);
  REQUIRE(test_sim.nurses[10] == 98); // swapped

  float dom = test_sim.dominance_interaction(1.f, 2.f);
  REQUIRE(dom < 1.f);
  dom = test_sim.dominance_interaction(1.f, 1.f);
  REQUIRE(dom == 0.5f); // (1 / (1 + exp(0)) = 1/2

  // update nurse
  test_sim.colony[0].set_fat_body(1.f);
  test_sim.update_nurse(&test_sim.colony[0]);
  CHECK(test_sim.colony[0].get_fat_body() == 1.f);
  test_sim.t = 1.f;
  test_sim.update_nurse(&test_sim.colony[0]);
  CHECK(test_sim.colony[0].get_fat_body() < 1.f);


  // forager sharing resources
  test_sim.colony[0].set_fat_body(1.f);
  size_t num_nurses = test_sim.nurses.size();
  test_sim.share_resources(&test_sim.colony[0]);
  size_t num_nurses_shared = test_sim.nurses.size();
  CHECK(num_nurses_shared == num_nurses - parameters.max_number_interactions);

  // update forager
  test_sim.colony[0].set_fat_body(1.f);
  test_sim.update_forager(&test_sim.colony[0]);
  CHECK(test_sim.colony[0].get_fat_body() > 1.f); // he received from crop/foraging

// update task
  test_sim.colony[0].set_fat_body(1000.f);
  test_sim.colony[0].set_current_task(forage);
  test_sim.colony[0].set_previous_task();
  test_sim.pick_task(&test_sim.colony[0]);
  CHECK(test_sim.colony[0].get_task() == nurse);

  test_sim.colony[0].set_fat_body(0.f);
  test_sim.colony[0].set_current_task(nurse);
  test_sim.colony[0].set_previous_task();
  test_sim.pick_task(&test_sim.colony[0]);
  CHECK(test_sim.colony[0].get_task() == forage);
}

TEST_CASE("TEST update colony") {

  params parameters;
  parameters.simulation_time = 100;
  rnd_t rndgen(42);

  Simulation test_sim(parameters);

  auto temp_t = test_sim.t;
  test_sim.update_colony();
  auto new_t = test_sim.t;
  REQUIRE(new_t >= temp_t);
}

TEST_CASE("TEST run simulation") {
  params parameters;
  parameters.simulation_time = 100;
  rnd_t rndgen(42);

  Simulation test_sim(parameters);

  test_sim.run_simulation();
  REQUIRE(test_sim.t >= parameters.simulation_time);
}

TEST_CASE("TEST freq") {
  params parameters;
  rnd_t rndgen(42);
  int id = 0;

  individual test_indiv;
  test_indiv.set_params(parameters,
                        id, rndgen);

  test_indiv.set_current_task(nurse);
  test_indiv.update_tasks(1.f);
  test_indiv.set_previous_task();
  test_indiv.set_current_task(forage);
  test_indiv.update_tasks(2.f);

  float freq_s = test_indiv.calc_freq_switches(0.f, parameters.simulation_time);
  CHECK(freq_s == 1.f); // switches all the way!
}


TEST_CASE("TEST individual") {

  params parameters;
  rnd_t rndgen(42);
  int id = 0;

  individual test_indiv;
  test_indiv.set_params(parameters,
                        id, rndgen);
  test_indiv.new_next_t(5.0);
  REQUIRE(test_indiv.get_next_t() == 5.0);

  test_indiv.set_fat_body(0.f);
  test_indiv.set_crop(5.f);
  test_indiv.process_crop_forager(0.5f, 1.f);
  CHECK(test_indiv.get_crop() == 2.5f);
  CHECK(test_indiv.get_fat_body() == 1.f);

  // check on update fatbody
  //fat body before  = 1.0f
  test_indiv.update_fatbody(1.f);
  CHECK(test_indiv.get_fat_body() < 1.f);

  test_indiv.reduce_crop(0.5f);
  CHECK(test_indiv.get_crop() == 2.f);

  test_indiv.reduce_crop(2.f); // crop is now empty!

  float new_food = test_indiv.handle_food(1.f, // food
                                          2.f,  // max_crop_size
                                          2,    // t
                                          0.5f); // handling_time

  CHECK(new_food == 0.0f); // remaining food is discarded
  CHECK(test_indiv.get_next_t() == 2.5f);
  CHECK(test_indiv.get_crop() == 1.f);

  new_food = test_indiv.handle_food(5.f, // food
                                    2.f,  // max_crop_size
                                    2,    // t
                                    0.5f); // handling_time
  CHECK(test_indiv.get_crop() == 2.f);
  CHECK(new_food == 4.f); // 5 - 2 * 0.5 = 4


  test_indiv.set_fat_body(100.f);
  test_indiv.decide_new_task(0.f,
                             rndgen,
                             1.f);
  CHECK(test_indiv.get_task() == nurse);
  CHECK(test_indiv.get_next_t() > 1.f);

  test_indiv.set_fat_body(0.f);
  test_indiv.decide_new_task(0.f,
                             rndgen,
                             1.f);
  CHECK(test_indiv.get_task() == forage);
  CHECK(test_indiv.get_next_t() == 1.f);

  test_indiv.update_tasks(1.f);
  CHECK(test_indiv.get_previous_t() == 1.f);
  auto temp_data = test_indiv.get_data().back();
  CHECK(std::get<0>(temp_data) == 1.f);
  CHECK(std::get<1>(temp_data) == test_indiv.get_task());
}
