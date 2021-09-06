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




}
