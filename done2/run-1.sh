#!/bin/bash
#SBATCH -c 16 -G 1
#SBATCH --output=out_%j.log
#SBATCH --error=err_%j.log

module load Python/Anaconda_v10.2019
source activate flow
python /home/aspotapova_2/point1.py