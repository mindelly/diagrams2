#!/bin/bash

#SBATCH --ntasks=1
#SBATCH -c 1
#SBATCH --output=out_%j.log
#SBATCH --error=err_%j.log
#SBATCH --array=10-15

y=2
NUP=`printf %d $SLURM_ARRAY_TASK_ID`
NDOWN=`printf %d $(($SLURM_ARRAY_TASK_ID-$y))`

./hubbard $NUM $NDOWN

# or ./hubbard $NUM 1