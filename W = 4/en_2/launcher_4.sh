#!/bin/bash

#SBATCH --ntasks=1
#SBATCH -c 1
#SBATCH --output=out_%j.log
#SBATCH --error=err_%j.log
#SBATCH --time=4-12
#SBATCH --array=40,60,80,100,140,180,200,240,280,300,320,360,400


#(Nx,Ny,Nup,Ndown,energy_file_name);
# y=20
# z=1
#L=`printf %d $(($SLURM_ARRAY_TASK_ID + $z))`
#NDOWN=`printf %d $(($y - $SLURM_ARRAY_TASK_ID - $z))`
L=`printf %d $SLURM_ARRAY_TASK_ID`

./hubbard $L 4 0.7 W4_07_trimers.txt
