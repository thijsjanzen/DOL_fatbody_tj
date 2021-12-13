#define CATCH_CONFIG_MAIN
#include "catch.h"

#include "../parameters.h"
#include "../simulation.h"
#include "../individual.h"
#include "../statistics.h"

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
  parameters.model_type = share_model::fair;
  rnd_t rndgen(parameters.mean_threshold, parameters.sd_threshold);

  std::unique_ptr<Simulation> test_sim = create_simulation(parameters);


  CHECK(test_sim->colony.size() == parameters.colony_size);

  test_sim->colony[0].set_previous_task();
  test_sim->colony[0].set_current_task(task::forage);

  // update nurse
  test_sim->colony[0].set_fat_body(1.f);
  test_sim->colony[0].update_nurse(test_sim->t);
  CHECK(test_sim->colony[0].get_fat_body() == 1.f);
  test_sim->t = 1.f;
  test_sim->colony[0].update_nurse(test_sim->t);
  CHECK(test_sim->colony[0].get_fat_body() < 1.f);


  // forager sharing resources
  test_sim->colony[0].set_fat_body(1.f);

// update task
  test_sim->colony[0].set_fat_body(1000.f);
  test_sim->colony[0].set_current_task(task::forage);
  test_sim->colony[0].set_previous_task();
  test_sim->colony[0].pick_new_task(0.f, rndgen, parameters);

  CHECK(test_sim->colony[0].get_task() == task::nurse);

  test_sim->colony[0].set_fat_body(0.f);
  test_sim->colony[0].set_current_task(task::nurse);
  test_sim->colony[0].set_previous_task();
  test_sim->colony[0].pick_new_task(0.f, rndgen, parameters);
  CHECK(test_sim->colony[0].get_task() == task::forage);
}

TEST_CASE("TEST update colony") {

  params parameters;
  parameters.simulation_time = 100;
  rnd_t rndgen(parameters.mean_threshold, parameters.sd_threshold);

  std::unique_ptr<Simulation> test_sim = create_simulation(parameters);


  auto temp_t = test_sim->t;
  test_sim->update_colony();
  auto new_t = test_sim->t;
  REQUIRE(new_t >= temp_t);
}

TEST_CASE("TEST run simulation") {
  params parameters;
  parameters.simulation_time = 100;
  rnd_t rndgen(parameters.mean_threshold, parameters.sd_threshold);

  std::unique_ptr<Simulation> test_sim = create_simulation(parameters);

  test_sim->run();
  REQUIRE(test_sim->t >= parameters.simulation_time);
}




TEST_CASE("TEST freq") {
  params parameters;
  rnd_t rndgen(parameters.mean_threshold, parameters.sd_threshold);

  individual test_indiv;
  test_indiv.initialize(parameters, rndgen,
                        fair_sharing_grouped);

  test_indiv.set_current_task(task::nurse);
  test_indiv.update_data(1.f);
  test_indiv.set_previous_task();
  test_indiv.set_current_task(task::forage);
  test_indiv.update_data(2.f);

  double freq_s = stats::calc_freq_switches(test_indiv, 0.f, 2.f);
  CHECK(freq_s == 1.f); // switches all the way!
}


TEST_CASE("TEST individual") {

  params parameters;
  rnd_t rndgen(parameters.mean_threshold, parameters.sd_threshold);
  int id = 0;

  individual test_indiv;

  test_indiv.initialize(parameters, rndgen, fair_sharing_grouped);

  test_indiv.set_fat_body(0.f);
  test_indiv.set_crop(5.f);
  test_indiv.process_crop();

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
                                          2,    // t
                                          0.5f); // handling_time

  CHECK(new_food == 0.0f); // remaining food is discarded
  CHECK(test_indiv.get_next_t() == 2.5f);
  CHECK(test_indiv.get_crop() == 1.f);

  new_food = test_indiv.handle_food(5.f, // food
                                    2,    // t
                                    0.5f); // handling_time
  CHECK(test_indiv.get_crop() == 2.f);
  CHECK(new_food == 4.f); // 5 - 2 * 0.5 = 4


  test_indiv.set_fat_body(100.f);
  test_indiv.decide_new_task(0.f,
                             rndgen,
                             1.f);
  CHECK(test_indiv.get_task() == task::nurse);
  CHECK(test_indiv.get_next_t() > 1.f);

  test_indiv.set_fat_body(0.f);
  test_indiv.decide_new_task(0.f,
                             rndgen,
                             1.f);
  CHECK(test_indiv.get_task() == task::forage);

  test_indiv.update_data(1.f);
  CHECK(test_indiv.get_previous_t() == 1.f);
  auto temp_data = test_indiv.get_data().back();
  CHECK(temp_data.current_task_  == test_indiv.get_task());
  CHECK(temp_data.t_ == 1.f);
}
