#!/bin/bash

#SBATCH --ntasks=1
#SBATCH -c 1
#SBATCH --output=out_%j.log
#SBATCH --error=err_%j.log
#SBATCH --array=10-15

NUM=`printf %d $SLURM_ARRAY_TASK_ID`
./hubbard $NUM 1
