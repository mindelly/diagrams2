#!/bin/bash
#SBATCH -c 4
#SBATCH --output=out_%j.log
#SBATCH --error=err_%j.log

module load Python/Anaconda_v10.2019
source activate flow
python /home/aspotapova_2/e4-calc1.py