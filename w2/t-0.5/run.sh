#!/bin/bash
#SBATCH -c 12
#SBATCH --output=out_%j.log
#SBATCH --error=err_%j.log
#SBATCH --time=45:00:00

module load Python/Anaconda_v10.2019
source activate flow
python /home/aspotapova_2/point20.py