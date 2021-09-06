//
//  simulation.h
//  dol_fatbody_tj
//
//  Created by Thijs Janzen on 07/04/2021.
//  Copyright © 2021 Thijs Janzen. All rights reserved.
//

#ifndef simulation_h
#define simulation_h

#include <vector>
#include <queue>
#include <tuple>

#include "individual.h"
#include "parameters.h"
#include "rand_t.h"

struct track_time {
   float time;
   individual* ind;

   track_time(individual* input) {
     ind = input;
     time = ind->get_next_t();
   }
};

struct cmp_time {
  bool operator()(const track_time& a, const track_time& b) {
    return a.time > b.time;
  }
};


struct Simulation {
  std::vector< individual > colony;

  std::vector<int> nurses;

  std::priority_queue< track_time,
                       std::vector<track_time>, cmp_time > time_queue;

  rnd_t rndgen;

  float t;
  sim_param p;
  int previous_time_recording;

  float brood_resources;



  Simulation(const sim_param& par) : p(par) {
    rndgen = rnd_t();
    rndgen.set_threshold_dist(p.get_ind_param().mean_threshold, p.get_ind_param().sd_threshold);
    colony = std::vector< individual >(p.get_meta_param().colony_size);

    t = 0.0;
    previous_time_recording = -1;

    brood_resources = 0.0;

    for (int i = 0; i < p.get_meta_param().colony_size; ++i) {

      colony[i].set_params(p.get_ind_param(), i, rndgen);

      double next_t = colony[i].get_next_t_threshold(t, rndgen);
      if(next_t < 0) next_t = 0.0;
      colony[i].go_nurse(next_t);
      nurses.push_back(colony[i].get_id());


      colony[i].update_tasks(t);
      time_queue.push(track_time(&colony[i]));
    }
  }

  bool check_time_interval(float t, float new_t, int time_interval) {
     if (time_interval <= 0) {
       return false;
     }

     int t1 = static_cast<int>(t / time_interval);
     int t2 = static_cast<int>(new_t / time_interval);

    if(t1 - t2 != 0) {
      return true;
    }
    return false;
   }

   void write_intermediate_output_to_file(std::string file_name,
                                       int t) {
     if (t == previous_time_recording) {
       return;
     }

     if (previous_time_recording == -1) { // first time writing to file
       std::ofstream out_file2(file_name.c_str());
       out_file2 << "time" << "\t"
                << "id" << "\t"
                << "task" << "\t"
                << "fat_body" << "\t"
                << "crop" << "\n";
       out_file2.close();
     }

     previous_time_recording = t;

     std::ofstream out_file(file_name.c_str(), std::ios::app);
     for (const auto& i : colony) {
       out_file << t << "\t"
                << i.get_id() << "\t"
                << i.get_task() << "\t"
                << i.get_fat_body() << "\t"
                << i.get_crop() << "\n";
     }
     out_file.close();
   }

  void remove_from_nurses(int id) {
    for (auto& i : nurses) {
      if (i == id) {
        i = nurses.back();
        nurses.pop_back();
        return;
      }
    }
    std::cout << "ERROR in remove_from_nurses";
    return;
  }

  float dominance_interaction(float fb_self, float fb_other) {
    return 1.f / (1.f + expf(fb_self - fb_other));
  }

  void share_resources(individual* focal_individual) {
    if (p.get_meta_param().model_type > 0) {
      // in model 0, there is NO sharing
      size_t num_interactions = std::min( static_cast<size_t>(p.get_meta_param().max_number_interactions),
                                          static_cast<size_t>(nurses.size()));

      std::vector< int > visited_nurses(num_interactions);

      for (size_t i = 0; i < num_interactions; ++i) {
        if (nurses.size() > 1) {
          int j = i + rndgen.random_number(static_cast<int>(nurses.size() - i));
          auto tmp = nurses[j];
          nurses[j] = nurses[i];
          nurses[i] = tmp;
        }

        int index_other_individual = nurses[i];

        float share_amount = 0.f;
        if (p.get_meta_param().model_type == 1) {
          share_amount = 1.f / num_interactions;
        }
        if (p.get_meta_param().model_type == 2) {
         // share_amount = dominance_interaction(focal_individual->get_dominance(),
         //                                      colony[index_other_individual].get_dominance());
          if (focal_individual->get_dominance() < colony[index_other_individual].get_dominance()) {
            share_amount = 1.0f;
          }
        }
        if (p.get_meta_param().model_type == 3) {
          share_amount = dominance_interaction(focal_individual->get_fat_body(),
                                              colony[index_other_individual].get_fat_body());
        }

        float to_share = share_amount * focal_individual->get_crop();

        double remainder  = colony[ index_other_individual].handle_food(to_share,
                                                    p.get_ind_param().proportion_fat_body_nurse,
                                                    p.get_ind_param().max_fat_body,
                                                    t,
                                                    p.get_ind_param().food_handling_time);
        visited_nurses[i] = colony[index_other_individual].get_id();
        //remove_from_nurses();

        float shared = to_share - remainder;
        brood_resources += shared * (1.0 - p.get_ind_param().proportion_fat_body_nurse);

        focal_individual->reduce_crop(shared);
      }

      for (auto nurse_id : visited_nurses) {
        remove_from_nurses(nurse_id);
      }
    }
  }

  void update_forager(individual* focal_individual) {
    focal_individual->update_fatbody(t);

    focal_individual->set_crop(p.get_env_param().resource_amount);
    focal_individual->process_crop(p.get_ind_param().proportion_fat_body_forager,
                                   p.get_ind_param().max_fat_body);

    share_resources(focal_individual);
    // the remainder in the crop is digested
    focal_individual->eat_crop(p.get_ind_param().max_fat_body);

    return;
  }

  void update_nurse(individual* focal_individual) {
    focal_individual->update_fatbody(t);
    return;
  }

  bool check_doubles() {
    int num_doubles = 0;
    for (size_t i = 0; i < nurses.size(); ++i) {
      for (size_t j = 0; j < nurses.size(); ++j) {
        if (i != j) {
          if (nurses[i] == nurses[j])
            num_doubles++;
        }
      }
    }
    if (num_doubles == 0) return false;
    return true;
  }


  void pick_task(individual* focal_individual) {

    if (focal_individual->get_previous_task() == forage) {
      // individual has returned from foraging
      // now has to decide if he goes foraging again.

      // the moment he goes foraging is picked with new_t:

      focal_individual->decide_new_task(t,
                                       rndgen,
                                       p.get_env_param().foraging_time);
    }
    if (focal_individual->get_previous_task() == nurse ||
        focal_individual->get_previous_task() == food_handling) {

      if (focal_individual->get_fat_body() -
          focal_individual->get_threshold() < 1e-2) {

        // individual is here because he has reached his threshold,
        // and goes foraging
        focal_individual->go_forage(t, p.get_env_param().foraging_time);
     } else {
        focal_individual->decide_new_task(t,
                                          rndgen,
                                          p.get_env_param().foraging_time);
      }
    }
  }

  void update_nurses(individual* focal_individual) {
    // individual started nursing:
    if (focal_individual->get_task() == nurse) {
      // but was not nursing:
      if (focal_individual->get_previous_task() != nurse) {
        nurses.push_back(focal_individual->get_id());
      }
    } else {
      // individual stopped nursing
      if (focal_individual->get_previous_task() == nurse) {
        // but was nursing
        remove_from_nurses(focal_individual->get_id());
      }
    }
  }




  void run_simulation() {

    while(t < p.get_meta_param().simulation_time) {

      auto next_ind = time_queue.top();
      time_queue.pop();
      if (next_ind.ind->get_next_t() > next_ind.time) {
        time_queue.push(track_time(next_ind.ind));
        continue;
      }

      auto focal_individual = next_ind.ind;

      double new_t = focal_individual->get_next_t();

      if (check_time_interval(t, new_t, p.get_meta_param().data_interval)) {
         write_intermediate_output_to_file(p.get_meta_param().output_file_name,
                                           t);
         std::cout << t << " " << nurses.size() << " " << colony.size() - nurses.size() << " " << brood_resources << "\n";
      }

      t = new_t;

      // update focal individual
      focal_individual->set_previous_task();

      if (focal_individual->get_task() == forage) {
        // update forager
        update_forager(focal_individual);
      } else {
        // nurse done nursing, or done handling food
        update_nurse(focal_individual);
      }

      pick_task(focal_individual);
      update_nurses(focal_individual);

      focal_individual->update_tasks(t);

      time_queue.push(track_time(focal_individual));
    }
    // end roll call:
    for (size_t i = 0; i < colony.size(); ++i) {
      colony[i].update_tasks(t);
    }
  }

  void write_ants_to_file(std::string file_name) {
    std::ofstream out(file_name.c_str());

    std::cout << "writing output to: " << file_name << "\n";

    out << "ID" << "\t" << "time" << "\t" << "task" << "\t" << "fat_body" << "\n";

    int cnt = 0;
    for (const auto& i : colony) {
      for (auto j : i.get_data()) {
        out << i.get_id() << "\t" << std::get<0>(j) << "\t"
            << std::get<1>(j) << "\t" << std::get<2>(j) << "\n"; // t, task, fat_body
      }
      cnt++;
    }
    out.close();
    return;
  }

  double calculate_gautrais() {
    std::vector<double> f_values(colony.size());
    int cnt = 0;
    for (const auto& i : colony) {
      double c = i.calc_freq_switches();

      f_values[cnt] = 1 - 2 * c;
      cnt++;
    }
    double avg_f = std::accumulate(f_values.begin(), f_values.end(), 0.0) *
                   1.0 / f_values.size();
    return avg_f;
  }

  double calculate_duarte() {
    std::vector<double> q(colony.size());
    std::vector<double> p(colony.size());
    size_t cnt = 0;
    size_t num_switches = 0;

    for (const auto& i : colony) {
      q[cnt] = 1 - i.calc_freq_switches();
      p[cnt] = i.count_p(num_switches);
      cnt++;
    }
    double q_bar = std::accumulate(q.begin(), q.end(), 0.0) *
                    1.0 / q.size();
    // now we need p1 ^ 2 and p2 ^ 2
    double p1 = std::accumulate(p.begin(), p.end(), 0.0) *
                      1.0 / num_switches;
    double p2 = 1 - p1;
    q_bar = q_bar / (p1 * p1 + p2 * p2);

    double D =  q_bar - 1;
    return D;
  }

  std::tuple<double, double, double> calculate_gorelick() {
    // HARDCODED 2 TASKS !!!
    std::vector<std::vector<double>> m(colony.size(), std::vector<double>(2, 0));
    // calculate frequency per individual per task
    auto sum = 0.0;
    for (size_t i = 0; i < colony.size(); ++i) {
      m[i] = colony[i].calculate_task_frequency(p.get_meta_param().simulation_time);
      sum += m[i][0] + m[i][1];
    }

    auto mult = 1.0 / sum;

    std::vector<double> pTask(2, 0.0);
    std::vector<double> pInd(m.size(), 0.0);

    for(size_t i = 0; i < m.size(); ++i) {
      for (size_t j = 0; j < 2; ++j) {
        m[i][j] *= mult; // normalize

        pTask[j] += m[i][j];
        pInd[i] += m[i][j];
      }
    }

    // calculate Hx, marginal entropy
     double Hx = 0;
     for (size_t i = 0; i < pTask.size(); ++i) {
       if (pTask[i] != 0.0) {
           Hx += pTask[i] * log(pTask[i]);
       }
     }

     Hx *= -1;

    // Calculate marginal entropy for individuals
    double Hy = 0.0;
    for (size_t i = 0; i < pInd.size(); ++i) {
           if (pInd[i] > 0) {
               Hy += pInd[i] * log(pInd[i]); // Again, this is Shannon's equation, but not yet mulpiplied by -1...
           }
    }
    Hy *= -1; // ...and here, again, multiplied by -1

   // calculate Ixy, mutual entropy
   double Ixy = 0;
   for (size_t i = 0; i < m[0].size(); ++i) {
     for (size_t j = 0; j < m.size(); ++j) {

       auto x = m[j][i];
       if (x != 0) {
        Ixy += x * log(x / (pTask[i] * pInd[j]));
       }
     }
   }

    double div_into_tasks = Ixy / Hx;
    double div_across_indivs = Ixy / Hy;
    double sim_div = Ixy / (sqrt(Hx * Hy));
    return std::make_tuple(div_into_tasks, div_across_indivs, sim_div);
  }


  void write_dol_to_file(const std::vector<double>& params,
                         const std::string& file_name) {

    std::cout << "writing dol to: " << file_name << "\n";

    std::ofstream out(file_name.c_str());

    for (auto i : params) {
      out << i << " ";
      std::cout << i << " ";
    }

    double gautrais = calculate_gautrais();
    double duarte = calculate_duarte();
    auto gorelick_stats = calculate_gorelick();

    std::cout << "DoL:\n";
    std::cout << "Gautrais 2000: " << gautrais << "\n";
    std::cout << "Duarte 2012  : " << duarte   << "\n";
    std::cout << "Gorelick 2004: ";

    out << gautrais << " " << duarte << " ";


    double div_tasks = std::get<0>(gorelick_stats);
    std::cout << div_tasks << " ";
    out << div_tasks << " ";

    double div_indiv = std::get<1>(gorelick_stats);
    std::cout << div_indiv << " ";
    out << div_indiv << " ";

    double div_both = std::get<2>(gorelick_stats);
    std::cout << div_both << "\n";
    out << div_both << "\n";

    out.close();
  }



};

#endif /* simulation_h */
