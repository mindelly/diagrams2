#!/bin/bash

#SBATCH --ntasks=1
#SBATCH -c 1
#SBATCH --output=out_%j.log
#SBATCH --error=err_%j.log
#SBATCH --time=6-12



# SBATCH --array=1-5
# (Nx,Ny,Nup,Ndown,energy_file_name);
# y=20
# z=1
# L=`printf %d $(($SLURM_ARRAY_TASK_ID + $z))`
# NDOWN=`printf %d $(($y - $SLURM_ARRAY_TASK_ID - $z))`

# W=`printf %d $SLURM_ARRAY_TASK_ID`
# NDOWN=`printf %d $SLURM_ARRAY_TASK_ID//2+1`

./hubbard 40 5 1.0 7 14 -7.0 W5_1_energies_L_40.txt W5_1_superconductive_L_7_14.txt
