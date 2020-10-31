#!/bin/bash
#SBATCH -c 6
#SBATCH --output=out_%j.log
#SBATCH --error=err_%j.log
#SBATCH --time=40:00:00

module load Python/Anaconda_v10.2019
source activate flow
python /home/aspotapova_2/point34.py