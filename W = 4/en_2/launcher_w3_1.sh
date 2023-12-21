#!/bin/bash

#SBATCH --ntasks=1
#SBATCH -c 1
#SBATCH --output=out_%j.log
#SBATCH --error=err_%j.log
#SBATCH --time=4-12


# SBATCH --array=2,3
#(Nx,Ny,Nup,Ndown,energy_file_name);
# y=20
# z=1
# L=`printf %d $(($SLURM_ARRAY_TASK_ID + $z))`
# NDOWN=`printf %d $(($y - $SLURM_ARRAY_TASK_ID - $z))`

# W=`printf %d $SLURM_ARRAY_TASK_ID`
# NDOWN=`printf %d $SLURM_ARRAY_TASK_ID//2+1`

# printf "W${SLURM_ARRAY_TASK_ID}_energies.txt"

./hubbard 40 3 0.2 40 20 W3_02_energies_L_40.txt W3_02_superconductive_L_40_40_20.txt
