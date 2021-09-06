#define CATCH_CONFIG_MAIN
#include "catch.h"

#include "../parameters.h"
#include "../ant_queen.h"
#include "../rand_t.h"
#include "../colony.h"
#include "../simulation.h"
#include "../output_storage.h"

#include <fstream>
#include <string>

TEST_CASE("TEST output", "[output]") {
  params temp_params;
  temp_params.pop_size = 5;

  colony test_colony(make_starting_queen(temp_params));
  size_t id = 0;
  for (size_t i = 0; i < 2; ++i) {
   auto new_genome = ant_genome(temp_params.num_age_classes,
                                 std::vector< float >(loci::max_num, 0.5f));

    test_colony.workers.push_back(ant_worker(new_genome,
                                             1.f, id));
  }

  auto results = calc_mean_colony(test_colony);
  REQUIRE(results.size() == temp_params.num_age_classes);
  for (auto i : results) {
    for (auto j : i) {
      CHECK(j == 0.5f);
    }
  }

  calculate_average(results, 5);
  for (auto i : results) {
    for (auto j : i) {
      CHECK(j == 0.1f);
    }
  }

  auto mean_queen_genome = get_mean_queen_genome(test_colony.queen);
  REQUIRE(mean_queen_genome.size() == test_colony.queen.chrom_one.chromosome.size());

  add_to_time_slice(results, mean_queen_genome);
  for (auto i : results) {
    for (auto j : i) {
      CHECK(j == 0.6f);
    }
  }


  temp_params.pop_size = 5;
  simulation test_simulation(temp_params);
  auto new_genome = ant_genome(temp_params.num_age_classes, std::vector< float >(loci::max_num, 0.5f));


  for (auto& i : test_simulation.population) {
    size_t id = 0;
    for (size_t j = 0; j < 3; ++j) {
      i.workers.push_back(ant_worker(new_genome, 0.f, id));
    }
  }

 // test_simulation.update_colonies();
  test_simulation.output.store(test_simulation.population);

  CHECK(test_simulation.output.mean_fatbody_nurse.back()   == 0.f);
  CHECK(test_simulation.output.mean_fatbody_forager.back() == 0.f);
  CHECK(test_simulation.output.mean_fatbody_queen.back()   == 10.f);
  CHECK(test_simulation.output.brood_size.back() == 0.f);
  CHECK(test_simulation.output.colony_size.back() == 3);

  test_simulation.output.display_to_screen(1);
  test_simulation.output.display_age_screen(1);

  test_simulation.parameters.max_t = 3;
  test_simulation.parameters.measure_interval = 1;
  test_simulation.run_simulation();
  CHECK(test_simulation.output.brood_size.size() == 1 + test_simulation.parameters.max_t);

  // we have now written genomes and output to file.
  std::ifstream infile(temp_params.genome_file_name);
  std::vector< mean_genome > found;
  std::string line;
  size_t cnt = 0;
  while(getline(infile, line)) {
    cnt++;
  }
  CHECK(cnt == test_simulation.parameters.max_t);

  infile.close();

  std::ifstream detailfile(temp_params.details_file_name);
  std::string garbage;
  for (size_t i = 0; i < 6; ++i) detailfile >> garbage;
  size_t cnt2 = 0;
  while(!detailfile.eof()) {
    int t, ID, fb, crop, age, is_foraging;
    detailfile >> t;
    detailfile >> ID;
    detailfile >> fb;
    detailfile >> crop;
    detailfile >> age;
    detailfile >> is_foraging;
    if (t == 0 && ID == -1) {
      CHECK(fb == temp_params.max_fat_body);
      CHECK(crop == temp_params.max_crop_size);
      CHECK(age == 0);
      CHECK(is_foraging == 0);
    }
    cnt2++;
    if (cnt2 > 10) {
      break;
    }
  }
  detailfile.close();



}




TEST_CASE("TEST simulation", "[simulation]") {
  params temp_params;
  temp_params.pop_size = 5;
  simulation test_simulation(temp_params);
  REQUIRE(test_simulation.population.size() == 5);

  // test sample ant queen

  test_simulation.population[0].brood = 2;
  test_simulation.population[1].brood = 1;

  std::vector<size_t> brood_items = std::vector< size_t >(5, 0);
  brood_items[0] = 2;
  brood_items[1] = 1;
  std::vector<size_t> dead_colonies = {2, 3, 4};
  test_simulation.replace_colonies(dead_colonies, brood_items);

  REQUIRE(test_simulation.population[2].ID == 5);
  REQUIRE(test_simulation.population[3].ID == 6);
  REQUIRE(test_simulation.population[4].ID == 7);

  REQUIRE(test_simulation.population[0].brood == 0);
  REQUIRE(test_simulation.population[1].brood == 0);
}



TEST_CASE("TEST update_colony", "[colony]") {
  params temp_params;
  rnd_t temprand(42, 10.f, 0.1f);

  colony test_colony(make_starting_queen(temp_params));

  temp_params.forage_risk = 0.f;

  auto new_genome = ant_genome(temp_params.num_age_classes,
                                  std::vector< float >(loci::max_num, 0.0f));
  size_t id = 0;
  auto forager = ant_worker(new_genome, 0.f, id);
  test_colony.workers.push_back(forager);
  auto nurse   = ant_worker(new_genome, 100.f, id);
  test_colony.workers.push_back(nurse);

  test_colony.update_colony(temp_params, temprand);

  REQUIRE(test_colony.workers[0].is_foraging == true);
  REQUIRE(test_colony.workers[1].is_foraging == false);
}


TEST_CASE("TEST colony update nurses", "[colony]") {
  params temp_params;
  rnd_t temprand(42, 3.f, 0.1f);

  colony test_colony(make_starting_queen(temp_params));
  test_colony.queen.crop = 0.f;
  size_t id = 0;
  std::vector< size_t > nurses;
  for (size_t i = 0; i < 2; ++i) {



    auto new_genome = ant_genome(temp_params.num_age_classes,
                                 std::vector< float >(loci::max_num, 0.5f));

    test_colony.workers.push_back(ant_worker(new_genome,
                                             1.f, id));
    test_colony.workers.back().is_foraging = false;
    test_colony.workers.back().avg_expression.chromosome[ 0 ][ beta] = 0.f;
    test_colony.workers.back().crop = 1.f;
    nurses.push_back(i);
  }

  test_colony.update_nurses(nurses, temp_params);
  REQUIRE(test_colony.queen.crop == 1.f); // received 0.5f from each nurse
}

TEST_CASE("TEST colony update foragers", "[colony]") {
  params temp_params;
  rnd_t temprand(42, 3.f, 0.1f);

  colony test_colony(make_starting_queen(temp_params));
  size_t id = 0;
  for (size_t i = 0; i < 4; ++i) {
    std::vector< float > markers;
    for (size_t j = 0; j < loci::max_num; ++j) {
      markers.push_back(temprand.uniform());
    }
    if (i == 0) markers = std::vector<float>(loci::max_num, 0.f);
    auto new_genome = ant_genome(temp_params.num_age_classes, markers);

    test_colony.workers.push_back(ant_worker(new_genome,
                                             0.f, id));
  }
  std::vector< size_t > dead_foragers;
  std::vector< size_t > foragers(1, 0); // first individual is forager
  std::vector< size_t > nurses;
  for (size_t i = 0; i < 3; ++i) {
    nurses.push_back(i + 1); // first is forager, rest is nurse.
  }


  temp_params.forage_bounty_mean = 3.f;
  temp_params.forage_bounty_sd = 0.f;
  temp_params.forage_risk = 0.f;


  test_colony.update_foragers(dead_foragers,
                              foragers,
                              nurses,
                              temp_params,
                              temprand);

  // now each nurse should have 1.f in the crop
  for (auto i : nurses) {
    REQUIRE(test_colony.workers[i].crop == 1.f);
  }

  // now we kill the worker
  temp_params.forage_risk = 1.0f;
  test_colony.update_foragers(dead_foragers,
                              foragers,
                              nurses,
                              temp_params,
                              temprand);
  // now each nurse should still have 1.f in the crop
  for (auto i : nurses) {
    REQUIRE(test_colony.workers[i].crop == 1.f);
  }
  REQUIRE(dead_foragers.size() == 1);

  // clean up of dead foragers
  test_colony.clean_up_dead_foragers(dead_foragers);
  dead_foragers.clear();
  REQUIRE(test_colony.workers.size() == 3);

  // now with two dead foragers
  foragers = {0, 1};
  nurses   = {2};
  test_colony.update_foragers(dead_foragers,
                              foragers,
                              nurses,
                              temp_params,
                              temprand);
  
  test_colony.clean_up_dead_foragers(dead_foragers);
  REQUIRE(test_colony.workers.size() == 1);
}



TEST_CASE("TEST colony feeding", "[colony]") {
  params temp_params;
  rnd_t temprand(42, 3.f, 0.1f);

  colony test_colony(make_starting_queen(temp_params));

  CHECK(test_colony.brood == 0);
  CHECK(test_colony.brood_resources == 0.f);
  CHECK(test_colony.queen_is_alive == true);

  float brood_resources = 100.f;
  float brood = 10;
  test_colony.workers.clear();
  test_colony.brood = brood;
  test_colony.brood_resources = brood_resources;
  test_colony.update_brood(1e6f, 0.01, temprand); // threshold is 1M, so need wayy too much to survive
  CHECK(test_colony.workers.size() == 0);


  test_colony.brood = brood;
  test_colony.brood_resources = brood_resources;
  test_colony.update_brood(0.f, 0.01, temprand); // threshold is 0, so need more than 0 to survive
  CHECK(test_colony.workers.size() == brood);


  // check survival of workers and queen
  // there are 10 workers
  for (size_t i = 0; i < test_colony.workers.size(); ++i) {
    if (i < 5) {
      test_colony.workers[i].fat_body = 10.f;
    } else {
      test_colony.workers[i].fat_body = 0.f;
    }
  }

  test_colony.queen.fat_body = 10.f;

  test_colony.survival(5.0, temprand, 0.f, 200);

  REQUIRE(test_colony.workers.size() == 5);
  REQUIRE(test_colony.queen_is_alive == true);

  test_colony.queen.fat_body = -10.f;
  test_colony.survival(0.0, temprand, 0.f, 200);
  REQUIRE(test_colony.workers.size() == 5);
  REQUIRE(test_colony.queen_is_alive == false);
}

TEST_CASE("TEST colony generate offspring", "[colony]") {
  params temp_params;
  rnd_t temprand(42, 3.f, 0.1f);

  colony test_colony(make_starting_queen(temp_params));

  auto prince = test_colony.generate_drone(temprand, 1.f, 0.f);
  REQUIRE(prince != test_colony.queen.chrom_one);
  REQUIRE(prince != test_colony.queen.chrom_two);
  REQUIRE(prince != test_colony.queen.stored_sperm);

  auto other_queen = test_colony.generate_ant_queen(prince,
                                                    temprand,
                                                    temp_params.max_fat_body, // max fat body
                                                    temp_params.max_crop_size,
                                                    1.0f, // mutate prob
                                                    0.f); // recom prob


  REQUIRE(other_queen != test_colony.queen);
}

TEST_CASE("TEST operators", "[colony]") {
  params temp_params;
  rnd_t temprand(42, 3.f, 0.1f);

  colony test_colony(make_starting_queen(temp_params));
  size_t id = 0;
  for (size_t i = 0; i < 5; ++i) {
    std::vector< float > markers;
    for (size_t j = 0; j < loci::max_num; ++j) {
      markers.push_back(temprand.uniform());
    }
    auto new_genome = ant_genome(temp_params.num_age_classes, markers);

    test_colony.workers.push_back(ant_worker(new_genome,
                                             10.f, id));
  }

  // now we have a colony with unique workers, let's move it around

  // copy operators
  auto copy_colony = test_colony;
  REQUIRE(copy_colony == test_colony);
  auto other_copy_colony(test_colony);
  REQUIRE(copy_colony == other_copy_colony);

  // move operators
  auto moved_colony = std::move(copy_colony);
  REQUIRE(moved_colony == test_colony);
  auto other_moved_colony(std::move(other_copy_colony));
  REQUIRE(other_moved_colony == test_colony);
}





TEST_CASE("TEST ant worker", "[ant worker]") {
  params temp_params;
  rnd_t temprand(42, 3.f, 0.1f);

  size_t ID = 1;
  ant_queen focal_queen = make_starting_queen(temp_params);
  ant_worker focal_worker(focal_queen.chrom_one,
                          focal_queen.chrom_two,
                          0.f, ID);

  CHECK(focal_worker.fat_body == 0.f);
  CHECK(focal_worker.crop == 0.f);
  CHECK(focal_worker.age == 0);

  // food processing testing, max crop
  float food = 5.f;
  float max_crop_size = 3.f;
  focal_worker.receive_food(food, max_crop_size);
  CHECK(focal_worker.crop == max_crop_size);
  CHECK(food == 2.f);

  // normal processing
  focal_worker.crop = 0.f;
  food = 2.f;
  max_crop_size = 5.f;
  focal_worker.receive_food(food, max_crop_size);
  CHECK(focal_worker.crop == 2.f);
  CHECK(food == 0.f);

  focal_worker.digest_crop(10.f);
  CHECK(focal_worker.crop == 0.f);
  CHECK(focal_worker.fat_body == 2.f);

  // process food upon return
  focal_worker.is_foraging = true;
  focal_worker.avg_expression.chromosome[0][0] = 1.f; // store everything
  focal_worker.crop = 5.f;
  focal_worker.fat_body = 0.f;
  focal_worker.increase_fat_body(10.f);

  CHECK(focal_worker.fat_body == 5.f);
  CHECK(focal_worker.crop == 0.f);

  // survival tests
  focal_worker.fat_body = 0.f;
  REQUIRE(focal_worker.survival(5.0, temprand, temp_params.maintenance, temp_params.num_age_classes) == false);
  focal_worker.decide_foraging(temprand);
  REQUIRE(focal_worker.is_foraging == true);

  focal_worker.fat_body = 10.f;
  REQUIRE(focal_worker.survival(5.0, temprand, temp_params.maintenance, temp_params.num_age_classes) == true);
  focal_worker.decide_foraging(temprand);
  REQUIRE(focal_worker.is_foraging == false);

}

TEST_CASE("TEST freq") {
  sim_param params;
  rnd_t rndgen(42);
  int id = 0;

  individual test_indiv;
  test_indiv.set_params(params.get_ind_param(),
                        id, rndgen);

  test_indiv.set_current_task(nurse);
  test_indiv.update_tasks(1.f);
  test_indiv.set_previous_task();
  test_indiv.set_current_task(forage);
  test_indiv.update_tasks(2.f);

  float freq_s = test_indiv.calc_freq_switches();
  CHECK(freq_s == 1.f); // switches all the way!
}





TEST_CASE("TEST reproduce ant queen", "[ant queen]")
{
  params temp_params;
  ant_queen focal_queen = make_starting_queen(temp_params);

  focal_queen.psi_gene[0] = 1.0f;
  focal_queen.fat_body = 10.f;
  size_t num_eggs = focal_queen.produce_brood(1.f);
  CHECK(focal_queen.fat_body == 0.f);
  CHECK(num_eggs == 10);


  // now generate a worker!
  rnd_t temprand(42, 0.5f, 0.1f);
  ant_worker temp_worker = focal_queen.generate_worker(1.0f, // prob of mutation
                                     0.f,
                              temprand);

  REQUIRE(focal_queen.chrom_one    != temp_worker.avg_expression);
  REQUIRE(focal_queen.chrom_two    != temp_worker.avg_expression);
  REQUIRE(focal_queen.stored_sperm != temp_worker.avg_expression);

  CHECK(temp_worker.age == 0);
  CHECK(temp_worker.crop == 0.f);
  CHECK(temp_worker.fat_body == 0.f);
  CHECK(temp_worker.is_foraging == false);

  ant_genome drone = focal_queen.generate_drone(temprand, 1.0f, 0.5f);
  REQUIRE(drone != focal_queen.chrom_one);
  REQUIRE(drone != focal_queen.chrom_two);
  REQUIRE(drone != focal_queen.stored_sperm);

  ant_queen other_queen = focal_queen.generate_new_queen(drone,
                                                         temprand,
                                                         0.f,
                                                         0.f,
                                                         1.f,
                                                         0.5f);
  REQUIRE(other_queen != focal_queen);
}



TEST_CASE("TEST food ant queen", "[ant queen]")
{
  params temp_params;
  ant_queen focal_queen = make_starting_queen(temp_params);

  REQUIRE(focal_queen.chrom_one == focal_queen.chrom_two);
  REQUIRE(focal_queen.age == 0);

  // food tests

  focal_queen.fat_body = 0.f;
  focal_queen.crop = 0.f;
  float food = 0.1f;
  float max_crop_size = 2.f;
  focal_queen.receive_food(food, max_crop_size);
  CHECK(focal_queen.crop == 0.1f);
  focal_queen.crop = 0.f;
  food = 5.f;
  focal_queen.receive_food(food, max_crop_size);
  CHECK(food == 3.f); // can only receive 2 due to max crop size
  CHECK(focal_queen.crop == max_crop_size);

  focal_queen.digest_crop(10.f); // digest with unlimited fb
  CHECK(focal_queen.fat_body == max_crop_size); // fb should be previous crop size
  CHECK(focal_queen.crop == 0.0); // crop is emptied now

  focal_queen.receive_food(food, max_crop_size); // receive some more food
  focal_queen.digest_crop(max_crop_size);        // set max fat body to crop size (queen fb = max_crop_size + crop = max_crop_size, so she can not eat all)
  CHECK(focal_queen.fat_body == max_crop_size);
  CHECK(focal_queen.crop == 0.0);

  // check survival
  focal_queen.fat_body = 0.f;
  rnd_t temprand(42, 0.5f, 0.1f);
  CHECK(focal_queen.survival(5, temprand, temp_params.maintenance, temp_params.num_age_classes) == false);

  focal_queen.fat_body = 10.f;
  CHECK(focal_queen.survival(5, temprand, temp_params.maintenance, temp_params.num_age_classes) == true);

<<<<<<< Updated upstream
}
=======
  new_food = test_indiv.handle_food(5.f, // food
                                    0.5f, // conversion_rate
                                    2.f,  // max_fat_body
                                    2,    // t
                                    0.5); // handling_time
  CHECK(test_indiv.get_fat_body() == 2.f);
  CHECK(new_food == 4.f); // 5 - 2 * 0.5 = 4

>>>>>>> Stashed changes



TEST_CASE("TEST params")
{
  params temp_params;
  REQUIRE( temp_params.num_age_classes == 20);
  // add more tests?
}

TEST_CASE("TEST genome")
{
  size_t num_age_classes = 5;
  ant_genome test_genome(num_age_classes, std::vector< float >(4, 0.5f));
  ant_genome test_genome2(num_age_classes);
  CHECK(test_genome == test_genome2);
  REQUIRE( test_genome.chromosome.size() == num_age_classes);
  REQUIRE( test_genome.chromosome[0][0] == 0.5f);
  
  float test_allele = 0.5f;
  float sd_mutation = 0.1f;
  float threshold = 0.1f;
  size_t seed = 42;
  rnd_t temprand(seed, threshold, sd_mutation);

  mutate_allele(test_allele, temprand);
  REQUIRE( test_allele != 0.5f);

  std::vector< float > alleles(1000, 0.5f);
  for (auto& i : alleles) {
    mutate_allele(i, temprand);
  }
  float mean_alleles = std::accumulate(alleles.begin(), alleles.end(), 0.f);
  mean_alleles *= 1.0 / alleles.size();
  // mean should still be 0.0;
  REQUIRE( mean_alleles < 0.52f);
  REQUIRE( mean_alleles > -0.48f);

  // mutate entire chromosome
  ant_genome mutated_chrom = mutate_chromosome(test_genome, 1.0, temprand);
  for (int i = 0; i < mutated_chrom.chromosome.size(); ++i) {
    for (int j = 0; j < mutated_chrom.chromosome[i].size(); ++j) {
      CHECK( mutated_chrom.chromosome[i][j] != test_genome.chromosome[i][j]);
    }
  }

  // recombine and mutate
  ant_genome genome_one(num_age_classes, std::vector< float >(4, 0.0f));
  ant_genome genome_two(num_age_classes, std::vector< float >(4, 1.0f));

  float mutate_prob = 0.f;
  float recom_prob = 0.5f;

  ant_genome recombined_genome = recombine_and_mutate(genome_one, genome_two, temprand, mutate_prob, recom_prob);

  float mean_genome = 0.0;
  for (const auto& i : recombined_genome.chromosome) {
    for (const auto& j : i) {
      mean_genome += j;
    }
  }
  mean_genome *= 1.0 / (recombined_genome.chromosome.size() * 4  );
  REQUIRE(mean_genome < 1.0);
  REQUIRE(mean_genome > 0.0);

  REQUIRE(genome_one != recombined_genome);
  REQUIRE(genome_two != recombined_genome);

  // test assignment
  ant_genome genome_before(num_age_classes, std::vector< float >(4, 0.0f));
  ant_genome genome_after = genome_before;

  REQUIRE(genome_after == genome_before);

  ant_genome genome_after_two(genome_before.chromosome);
  REQUIRE(genome_after_two == genome_before);
  REQUIRE(genome_after_two == genome_after);
}

