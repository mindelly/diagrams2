#!/bin/bash
#SBATCH -c 10
#SBATCH --output=out_%j.log
#SBATCH --error=err_%j.log

module load Python/Anaconda_v10.2019
source activate flow
python /home/aspotapova_2/test_for_more_eig.py