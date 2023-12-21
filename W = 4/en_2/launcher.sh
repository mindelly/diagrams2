#!/bin/bash

#SBATCH --ntasks=1
#SBATCH -c 5
#SBATCH --output=out_%j.log
#SBATCH --error=err_%j.log
#SBATCH --time=4-12


# (Nx,Ny,Nup,Ndown,energy_file_name);
# y=20
# z=1
# NUP=`printf %d $(($SLURM_ARRAY_TASK_ID + $z))`
# NDOWN=`printf %d $(($y - $SLURM_ARRAY_TASK_ID + $z))`
# L=`printf %d $SLURM_ARRAY_TASK_ID`

./hubbard 40 5 15 5 W5_L_40.txt
