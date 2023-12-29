#!/bin/bash

#SBATCH --ntasks=1
#SBATCH -c 1
#SBATCH --output=out_%j.log
#SBATCH --error=err_%j.log
#SBATCH --time=14-12
#SBATCH --array=10,11,12,13,14,15,16,17,18,19

# (Nx,Ny,Nup,Ndown,energy_file_name);
# y=20
# z=1
# L=`printf %d $(($SLURM_ARRAY_TASK_ID + $z))`
# NDOWN=`printf %d $(($y - $SLURM_ARRAY_TASK_ID - $z))`

NUP=`printf %d $SLURM_ARRAY_TASK_ID`
NDOWN=`printf %d $SLURM_ARRAY_TASK_ID`

./hubbard 40 4 1 $NUP $NDOWN -7.0 0 -1 1
