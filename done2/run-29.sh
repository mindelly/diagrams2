#!/bin/bash
#SBATCH -c 8
#SBATCH --output=out_%j.log
#SBATCH --error=err_%j.log

module load Python/Anaconda_v10.2019
source activate flow
python /home/aspotapova_2/point29.py
