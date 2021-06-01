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

  Simulation(const sim_param& par) : p(par) {
    rndgen = rnd_t();
    rndgen.set_threshold_dist(p.get_meta_param().threshold_mean,
                              p.get_meta_param().threshold_sd);

    colony = std::vector< individual >(p.get_meta_param().colony_size);

    t = 0.0;
    previous_time_recording = -1;

    for (int i = 0; i < p.get_meta_param().colony_size; ++i) {
      nurses.push_back(i);
      colony[i].set_params(p.get_ind_param(), i);
      colony[i].update_threshold(rndgen);
      time_queue.push( track_time(&colony[i]));
      colony[i].update_tasks(t);
    }
  }

  bool check_time_interval(float t, int time_interval) {
     if (time_interval <= 0) {
       return false;
     }

     int floored_time = static_cast<int>(t);
     if (floored_time % time_interval == 0) {
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

  void update_forager(individual* focal_individual) {
    focal_individual->set_crop(p.get_env_param().resource_amount);
    focal_individual->process_crop(p.get_ind_param().proportion_fat_body_forager);

    if (p.get_meta_param().model_type > 0) {

      // interactions
      size_t num_interactions = std::max( static_cast<size_t>(p.get_meta_param().max_number_interactions),
                                          static_cast<size_t>(nurses.size()));

      for (size_t i = 0; i < num_interactions; ++i) {
        int j = rndgen.random_number(nurses.size());
        int tmp = nurses[j];
        nurses[j] = nurses[i];
        nurses[i] = tmp;

        int index_other_individual = nurses[i];

        float share_amount = 0.f;
        if (p.get_meta_param().model_type > 1) {
              share_amount = dominance_interaction(focal_individual->get_fat_body(),
                                                   colony[index_other_individual].get_fat_body());
        } else {
          share_amount = rndgen.uniform();
        }

        colony[ index_other_individual ].receive_food(share_amount * focal_individual->get_crop(),
                                                      p.get_ind_param().proportion_fat_body_nurse);
        focal_individual->reduce_crop(share_amount * focal_individual->get_crop());
      }
    }

    if (focal_individual->get_crop() > 0) {
      focal_individual->receive_food(focal_individual->get_crop(),
                                     p.get_ind_param().proportion_fat_body_forager);
      focal_individual->set_crop(0.0); // just to be sure.
    }
  }


  void update_individual(individual* focal_individual) {
    focal_individual->update_fatbody(t);

    if (focal_individual->not_at_threshold()) {
      // in between updates, this individual has received food
      focal_individual->set_next_t_nurse();
      return; // done
    }

    // if the individual returned from foraging:
    if (focal_individual->get_task() == forage) {
      update_forager(focal_individual);

      focal_individual->update_threshold(rndgen);
      
      if (focal_individual->not_at_threshold()) {
        // start nursing
        focal_individual->set_current_task(nurse);
        nurses.push_back(focal_individual->get_id());
      } else { // go out foraging again
        focal_individual->set_current_task(forage);
        focal_individual->set_next_t_forager(p.get_env_param().foraging_time);
      }
      focal_individual->update_tasks(t);
      return;
    }

    if (focal_individual->get_task() == nurse) {
      // focal individual was nursing, and starts foraging
      // (note that the threshold has been reached, otherwise
      // it would've already been checked above.
      remove_from_nurses(focal_individual->get_id());

      focal_individual->set_next_t_forager(p.get_env_param().foraging_time);
      focal_individual->set_current_task(forage);

      focal_individual->update_tasks(t);
      return;
    }
  }

  void run_simulation() {

    while(t < p.get_meta_param().simulation_time) {
      auto focal_individual = time_queue.top().ind;
      time_queue.pop();
      t = focal_individual->get_next_t();

      if (check_time_interval(t, p.get_meta_param().data_interval)) {
         write_intermediate_output_to_file(p.get_meta_param().output_file_name,
                                           t);
      }

      update_individual(focal_individual);
      // re-add individual to queue
      time_queue.push(track_time(focal_individual));
    }
  }

  void write_ants_to_file(std::string file_name) {
    std::ofstream out(file_name.c_str());

    std::cout << "writing output to: " << file_name << "\n";

    out << "ID" << "\t" << "time" << "\t" << "task" << "\t" << "fat_body" << "\n";

    int cnt = 0;
    for (const auto& i : colony) {
      for (auto j : i.get_data()) {
        out << i.get_id() << " " << std::get<0>(j) << " " << std::get<1>(j) << " " << std::get<2>(j) << "\n"; // t, task, fat_body
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
