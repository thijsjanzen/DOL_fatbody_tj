//
//  Header.h
//  dol_fatbody_tj
//
//  Created by Thijs Janzen on 12/10/2021.
//  Copyright © 2021 Thijs Janzen. All rights reserved.
//

#ifndef statistics_h
#define statistics_h

#include <numeric>

namespace stats {

  double calculate_gautrais(const std::vector< individual>& colony,
                            float min_t, float max_t) {
    std::vector<double> f_values(colony.size());
    int cnt = 0;
    for (const auto& i : colony) {
      double c = i.calc_freq_switches(min_t, max_t);
      f_values[cnt] = 1.0 - 2.0 * c;
      cnt++;
    }
    return std::accumulate(f_values.begin(), f_values.end(), 0.0) *
                   1.0 / f_values.size();
  }

  double calculate_duarte(const std::vector< individual>& colony,
                          float min_t, float max_t) {
    std::vector<double> q(colony.size());
    std::vector<size_t> p(colony.size());
    size_t cnt = 0;
    size_t num_switches = 0;

    for (const auto& i : colony) {
      q[cnt] = 1 - i.calc_freq_switches(min_t, max_t);
      p[cnt] = i.count_p(min_t, max_t, num_switches);
      cnt++;
    }
    double q_bar = std::accumulate(q.begin(), q.end(), 0.0) *
                    1.0 / q.size();
    // now we need p1 ^ 2 and p2 ^ 2
    // TODO: check if num_switches == 0
    double p1 = std::accumulate(p.begin(), p.end(), 0.0) *
                      1.0 / num_switches;
    double p2 = 1 - p1;
    return q_bar / (p1 * p1 + p2 * p2) - 1;
  }

  std::tuple<double, double, double> calculate_gorelick(const std::vector< individual>& colony,
                                                        float min_t, float max_t) {
    // HARDCODED 2 TASKS !!!
    std::vector<std::vector<float>> m(colony.size(), std::vector<float>(2, 0.f));
    // calculate frequency per individual per task
    double sum = 0.0;
    for (size_t i = 0; i < colony.size(); ++i) {
      m[i] = colony[i].calculate_task_frequency(min_t, max_t);
      sum += double(m[i][0]) + double(m[i][1]);
    }

    double mult = 1.0 / sum;

    std::vector<double> pTask(2, 0.0);
    std::vector<double> pInd(m.size(), 0.0);

    for(size_t i = 0; i < m.size(); ++i) {
      for (size_t j = 0; j < 2; ++j) {
        m[i][j] *= mult; // normalize

        pTask[j] += static_cast<double>(m[i][j]);
        pInd[i]  += static_cast<double>(m[i][j]);
      }
    }

    // calculate Hy, marginal entropy
     double Hy = 0;
     for (size_t i = 0; i < pTask.size(); ++i) {
       if (pTask[i] != 0.0) {
           Hy += pTask[i] * log(pTask[i]);
       }
     }

     Hy *= -1;

    // Calculate marginal entropy for individuals
    double Hx = 0.0;
    for (size_t i = 0; i < pInd.size(); ++i) {
           if (pInd[i] > 0) {
               Hx += pInd[i] * log(pInd[i]); // Again, this is Shannon's equation, but not yet mulpiplied by -1...
           }
    }
    Hx *= -1; // ...and here, again, multiplied by -1

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

    double div_into_tasks = Ixy / Hy;  // in the original paper, Hx is reversed with Hy, but
                                       // we only get identical results to the paper assuming Hx = Hy.
    double div_into_indivs = Ixy / Hx;
    double sim_div = Ixy / (sqrt(Hy * Hx));
    return std::make_tuple(div_into_tasks, div_into_indivs, sim_div);
  }
}

namespace output {

   void write_dol_to_file(const std::vector< individual>& colony,
                          const std::vector< float>& param_values,
                          const std::string& file_name,
                          size_t num_repl,
                          float burnin,
                          float total_time) {

    std::cout << "writing dol to: " << file_name << "\n";
    std::ofstream out(file_name.c_str(), std::ios::app);

    // write parameter values to file
    out << num_repl << "\t";
    for (auto i : param_values) {
      out << i << "\t";
    }

    float min_t = burnin * total_time;
    float max_t = total_time;

    double gautrais		= stats::calculate_gautrais(colony, min_t, max_t);
    double duarte			= stats::calculate_duarte(colony, min_t, max_t);
    auto gorelick_stats		= stats::calculate_gorelick(colony, min_t, max_t);

    std::cout << "Gautrais 2002: " << gautrais << "\n";
    std::cout << "Duarte 2012  : " << duarte   << "\n";
    std::cout << "Gorelick 2004: ";

    out << gautrais << "\t" << duarte << "\t";


    double div_tasks = std::get<0>(gorelick_stats);
    std::cout << div_tasks << " ";
    out << div_tasks << "\t";

    double div_indiv = std::get<1>(gorelick_stats);
    std::cout << div_indiv << " ";
    out << div_indiv << "\t";

    double div_both = std::get<2>(gorelick_stats);
    std::cout << div_both << "\n";
    out << div_both << "\n";

    out.close();
  }

  void write_dol_headers(const std::vector<std::string>& param_names,
                         const std::string& file_name,
                         const std::string& window_file_name,
                         size_t data_interval) {

    std::ofstream out(file_name.c_str());
    out << "repl\t";
    for (auto i : param_names) {
      out << i << "\t";
    } out << "gautrais\tduarte\tgorelick_tasks\tgorelick_indiv\tgorelick_both\n";
    out.close();

    if (data_interval == 0) {
      std::ofstream out(window_file_name.c_str());
      out << "repl" << "\t" << "min_t" << "\t" << "max_t" << "\t" <<
                "gautrais\tduarte\tgorelick_tasks\tgorelick_indiv\tgorelick_both\n";
      out.close();
    }
  }

  void write_ants_to_file(const std::vector< individual>& colony,
                          std::string file_name,
                          size_t num_repl) {
    std::ofstream out(file_name.c_str());

    std::cout << "writing output to: " << file_name << "\n";

    out << "replicate" << "\t" << "ID" << "\t" << "time" << "\t" << "task" << "\t" << "fat_body" << "\n";

    int cnt = 0;
    for (const auto& i : colony) {
      for (auto j : i.get_data()) {
        out << num_repl << "\t" << i.get_id() << "\t" << j.t_ << "\t"
            << static_cast<int>(j.current_task_) << "\t" << j.fb_ << "\n"; // t, task, fat_body

      }
      cnt++;
    }
    out.close();
    return;
  }

  void write_dol_sliding_window(const std::vector< individual>& colony,
                                float window_size,
                                float window_step_size,
                                float simulation_time,
                                std::string file_name,
                                size_t num_repl) {

    std::ofstream out(file_name.c_str(), std::ios::app);
    std::cout << "writing windowed DoL output to: " << file_name << "\n";

    for (float max_t = window_size; max_t <= simulation_time; max_t += window_step_size) {
      float min_t = max_t - window_size;;
      double gautrais = stats::calculate_gautrais(colony, min_t, max_t);
      double duarte = stats::calculate_duarte(colony, min_t, max_t);
      auto gorelick_stats = stats::calculate_gorelick(colony, min_t, max_t);

      out << num_repl << "\t" << min_t << "\t" << max_t << "\t" <<
              gautrais << "\t" << duarte << "\t" <<
              std::get<0>(gorelick_stats) << "\t" <<
              std::get<1>(gorelick_stats) << "\t" << 
              std::get<2>(gorelick_stats) << "\n";
    }
    out.close();
  }
}

#endif /* Header_h */
