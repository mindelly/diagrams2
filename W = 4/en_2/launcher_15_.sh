#!/bin/bash

#SBATCH --ntasks=1
#SBATCH -c 5
#SBATCH --output=out_%j.log
#SBATCH --error=err_%j.log
#SBATCH --time=8-12



#(Nx,Ny,Nup,Ndown,energy_file_name);
# y=20
# z=1
# L=`printf %d $(($SLURM_ARRAY_TASK_ID + $z))`
# NDOWN=`printf %d $(($y - $SLURM_ARRAY_TASK_ID - $z))`

# W=`printf %d $SLURM_ARRAY_TASK_ID`
# NDOWN=`printf %d $SLURM_ARRAY_TASK_ID//2+1`

./hubbard 80 4 0.3 11 25 -5
