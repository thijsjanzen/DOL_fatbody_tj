#define CATCH_CONFIG_MAIN
#include "catch.h"

#include "../parameters.h"
#include "../simulation.h"
#include "../individual.h"

#include <fstream>
#include <string>

TEST_CASE("TEST individual") {

  sim_param params;
  rnd_t rndgen(42);
  int id = 0;

  individual test_indiv;
  test_indiv.set_params(params.get_ind_param(),
                        id, rndgen);
  test_indiv.new_next_t(5.0);
  REQUIRE(test_indiv.get_next_t() == 5.0);

  test_indiv.set_fat_body(0.f);
  test_indiv.set_crop(5.f);
  test_indiv.process_crop(0.5f, 1.f);
  CHECK(test_indiv.get_crop() == 2.5f);
  CHECK(test_indiv.get_fat_body() == 1.f);

  // check on update fatbody
  //fat body before  = 1.0f
  test_indiv.update_fatbody(1.f);
  CHECK(test_indiv.get_fat_body() < 1.f);

  test_indiv.reduce_crop(0.5f);
  CHECK(test_indiv.get_crop() == 2.f);

  test_indiv.eat_crop(1.f);
  CHECK(test_indiv.get_crop() == 0.f);
  CHECK(test_indiv.get_fat_body() == 1.f);

  float new_food = test_indiv.handle_food(1.f, // food
                         0.5f, // conversion_rate
                         2.f,  // max_fat_body
                         2,    // t
                         0.5); // handling_time

  CHECK(new_food == 0.0f); // remaining food is discarded
  CHECK(test_indiv.get_fat_body() == 1.5f);
  CHECK(test_indiv.get_next_t() == 2.5f);


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
