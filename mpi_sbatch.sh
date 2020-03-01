#!/bin/bash 
#SBATCH --time=00:01:00
##SBATCH --constraint=dwarves
#SBATCH --nodes=1 --ntasks-per-node=4
#SBATCH -o pt2.out

module load OpenMPI

mpirun -np $SLURM_NPROCS MPI_OpenMP
