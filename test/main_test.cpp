#define CATCH_CONFIG_MAIN
#include "catch.h"

#include "../parameters.h"
#include "../simulation.h"
#include "../individual.h"
#include "../statistics.h"

#include <fstream>
#include <string>

float get_share_amount(std::unique_ptr<Simulation>& sim,
                       const params& p,
                       rnd_t& rndgen) {
  sim->colony[0].set_fat_body(0.0f);
  sim->colony[0].set_dominance(0.1f);

  sim->colony[1].set_fat_body(1.f);
  sim->colony[1].set_dominance(0.2f);

  sim->colony[0].set_current_task(task::forage);
  sim->colony[0].set_current_task(task::nurse);

  std::vector< individual * > nurses;
  for (int i = 1; i < sim->colony.size(); ++i) {
    nurses.push_back(&sim->colony[i]);
  }

  sim->colony[0].set_crop(p.resource_amount);

  sim->colony[0].share_resources_grouped(0.f,
                                         nurses,
                                         p,
                                         rndgen);

  return sim->colony[0].get_crop();
}

TEST_CASE("TEST initialize simulation") {

  params parameters;
  parameters.colony_size = 2;
  rnd_t rndgen(parameters.mean_threshold, parameters.sd_threshold);

  parameters.model_type = share_model::no;

  std::unique_ptr<Simulation> test_sim1 = create_simulation(parameters);

  float result = get_share_amount(test_sim1,
                                  parameters,
                                  rndgen);
  CHECK(result == parameters.resource_amount);

  parameters.model_type = share_model::fair;
  std::unique_ptr<Simulation> test_sim2 = create_simulation(parameters);

  float result2 = get_share_amount(test_sim2,
                                  parameters,
                                  rndgen);
  CHECK(result2 == parameters.resource_amount * 0.5f);


  parameters.model_type = share_model::dominance;
  parameters.soft_max = 100;

  std::unique_ptr<Simulation> test_sim3 = create_simulation(parameters);

  float result3 = get_share_amount(test_sim3,
                                   parameters,
                                   rndgen);
  CHECK(result3 < parameters.resource_amount * 1e-3f);


  parameters.model_type = share_model::fat_body;
  std::unique_ptr<Simulation> test_sim4 = create_simulation(parameters);

  float result4 = get_share_amount(test_sim4,
                                   parameters,
                                   rndgen);
  CHECK(result4 < parameters.resource_amount * 1e-3f);




  // this is superficial testing, but better than nothing
  REQUIRE(test_sim1->colony.size() == test_sim2->colony.size());
  REQUIRE(test_sim2->colony.size() == test_sim3->colony.size());
  REQUIRE(test_sim3->colony.size() == test_sim4->colony.size());
}


TEST_CASE("TEST parameters") {

  // we test here the ability to extract parameters to record
  params parameters;
  auto s = "resource_amount,foraging_time,burnin,num_replicates";
  auto vec = parameters.split(s);
  REQUIRE(vec.size() == 4);
  CHECK(vec[0] == "resource_amount");
  CHECK(vec[1] == "foraging_time");
  CHECK(vec[2] == "burnin");
  CHECK(vec[3] == "num_replicates");

  auto vals = parameters.create_params_to_record(vec);
  CHECK(vals[0] == parameters.resource_amount);
  CHECK(vals[1] == parameters.foraging_time);
  CHECK(vals[2] == parameters.burnin);
  CHECK(vals[3] == parameters.num_replicates);
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

  test_indiv.set_current_task(task::forage);
  test_indiv.update_data(1.f);
  test_indiv.set_previous_task();
  test_indiv.set_current_task(task::nurse);
  test_indiv.update_data(2.f);

  double freq_s = stats::calc_freq_switches(test_indiv, 0.f, 2.f);
  // 0 = nurse, 1 = forage, 2 = nurse. 2/2 switches.
  CHECK(freq_s == 1.0f); // switches all the way!
}


TEST_CASE("TEST individual") {

  params parameters;
  rnd_t rndgen(parameters.mean_threshold, parameters.sd_threshold);

  individual test_indiv;

  test_indiv.initialize(parameters, rndgen, fair_sharing_grouped);

  test_indiv.set_fat_body(0.f);
  test_indiv.set_crop(1.f);
  test_indiv.process_crop();

  CHECK(test_indiv.get_crop() == 0.f); // crop is fully processed
  CHECK(test_indiv.get_fat_body() == 1.f); // all crop to fb

  // check on update fatbody
  //fat body before  = 1.0f
  test_indiv.update_fatbody(1.f);
  CHECK(test_indiv.get_fat_body() < 1.f);

  float new_food = test_indiv.handle_food(1.f, // food
                                          2,    // t
                                          0.5f); // handling_time

  CHECK(new_food == 0.0f); // remaining food is discarded
  CHECK(test_indiv.get_next_t() == 2.5f);
  CHECK(test_indiv.get_crop() == 1.f);

  new_food = test_indiv.handle_food(5.f, // food
                                    2,    // t
                                    0.5f); // handling_time

  CHECK(test_indiv.get_crop() == 6.f);
  CHECK(new_food == 0.f); // 5 - 2 * 0.5 = 4


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

TEST_CASE("TEST sharing") {
  params parameters;
  parameters.max_fat_body = 1.f; // for simplicity later on.

  rnd_t rndgen(parameters.mean_threshold, parameters.sd_threshold);

  std::vector<individual> indivs(2);
  // no sharing
  for(auto& i : indivs) {
    i.initialize(parameters, rndgen,
                 no_sharing_grouped); // sharing type doesn't matter here.
  }

  std::vector< individual*> nurses;
  for (int i = 1; i < 2; ++i) {
    nurses.push_back( &indivs[i] );
  }

  std::vector< ctype_ > share_amount =
              no_sharing_grouped(&indivs[0],
                                        nurses,
                                        parameters.soft_max,
                                        1);
  REQUIRE(share_amount[0] == 0.f);

  share_amount =
              fair_sharing_grouped(&indivs[0],
                                        nurses,
                                        parameters.soft_max,
                                        1);
  REQUIRE(share_amount[0] == 0.5f);

  share_amount =
                  dominance_sharing_grouped(&indivs[0],
                                            nurses,
                                            0, // 0 defaults to fair sharing
                                            1);
  REQUIRE(share_amount[0] == 0.5f);

  indivs[0].set_dominance(0.5f);
  indivs[1].set_dominance(0.5f);

  share_amount =
  dominance_sharing_grouped(&indivs[0],
                            nurses,
                            1,
                            1);

  REQUIRE(share_amount[0] == 0.5f);

  indivs[0].set_dominance(0.1f);
  indivs[1].set_dominance(0.4f);

  share_amount =
  dominance_sharing_grouped(&indivs[0],
                            nurses,
                            100, // s
                            1);
  
  REQUIRE(share_amount[0] == 1.0f);

  share_amount =
  dominance_sharing_grouped(&indivs[0],
                            nurses,
                            10, // s
                            1);

  REQUIRE(share_amount[0] > 0.4 / 0.5);

  // fb tests

  share_amount =
                  fatbody_sharing_grouped(&indivs[0],
                                            nurses,
                                            0, // 0 defaults to fair sharing
                                            1);
  REQUIRE(share_amount[0] == 0.5f);

  indivs[0].set_fat_body(0.5f);
  indivs[1].set_fat_body(0.5f);


  share_amount =
  fatbody_sharing_grouped(&indivs[0],
                            nurses,
                            1,
                            1);

  REQUIRE(share_amount[0] == 0.5f);

  indivs[0].set_fat_body(0.1f);
  indivs[1].set_fat_body(0.4f);

  share_amount = fatbody_sharing_grouped(&indivs[0],
                                          nurses,
                                          100, // s
                                          1);

  REQUIRE(share_amount[0] == 1.0f);

  share_amount =
  fatbody_sharing_grouped(&indivs[0],
                            nurses,
                            10, // s
                            1);

  REQUIRE(share_amount[0] > 0.4 / 0.5);
}

