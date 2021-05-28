# automate parameter exploration:
#
source("create_json.R")

# create json files:
resource_vals <- seq(1.0, 10.0, by = 0.25)

for (i in 1:length(resource_vals)) {
  create_json(json_file_name = paste0("json_", i, ".json"),
              resource_amount = resource_vals[i],
              dol_file_name = paste0("dol_", i, ".txt"),
              output_file_name = paste0("out_", i, ".txt"),
              model_type = 0)
}

# now all you need to do is create a bash script that
# explores i:

#### SLURM settings ####
# #SBATCH --ntasks=1
# #SBATCH --job-name=example
# #SBATCH --output=slurm.%j.out
# #SBATCH --time=0-01:00
# #SBATCH --mem=4048
# #SBATCH --array=1-100
#
# i=$SLURM_ARRAY_TASK_ID
# ./dol_exec json_$i.json
