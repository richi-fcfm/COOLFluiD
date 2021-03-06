#!/bin/bash

export DEPS=/data2/underbackup/lani/BENJAMIN/TEST_RAD/COOLFluiD_RADIATION/DEPS

export PATH=$DEPS/bin:$DEPS/mutation++/install/bin:$PATH
echo 'PATH is $PATH'
export LD_LIBRARY_PATH=$DEPS/lib:$DEPS/mutation++/install/lib:$LD_LIBRARY_PATH
echo 'LD_LIBRARY_PATH is $LD_LIBRARY_PATH'
export MPP_DATA_DIRECTORY=$DEPS/mutation++/data
echo 'MPP_DATA_DIRECTORY is $MPP_DATA_DIRECTORY'

#source ~/.bashrc
#CF2_OPENMPI

export CDIR=/data2/underbackup/lani/BENJAMIN/TEST_RAD/COOLFluiD_RADIATION/OPENMPI/optim/apps/Solver

cd $CDIR

####### Radiative properties computed from scratch

echo "Running FIREII with HSNB:    fire_viscous_HSNB_MC_fresh_limrange.CFcase"
mpirun -np 8 ./coolfluid-solver --scase /data2/underbackup/lani/BENJAMIN/TEST_RAD/COOLFluiD_RADIATION/plugins/RadiativeTransfer/testcases/FireIIDLR/fire_viscous_HSNB_MC_fresh_limrange.CFcase >& fire_viscous_HSNB_MC_fresh_limrange.CFcase.txt

echo "Running FIREII with PARADE:  fire_viscous_DLR_PARADE_fresh.CFcase"
mpirun -np 8 ./coolfluid-solver --scase /data2/underbackup/lani/BENJAMIN/TEST_RAD/COOLFluiD_RADIATION/plugins/RadiativeTransfer/testcases/FireIIDLR/fire_viscous_DLR_PARADE_fresh.CFcase >& fire_viscous_DLR_PARADE_fresh.CFcase.txt

killall -9 parade

echo "Running HUYGENS with HSNB:   huygens_HSNB_MC_fresh.CFcase"
mpirun -np 8 ./coolfluid-solver --scase /data2/underbackup/lani/BENJAMIN/TEST_RAD/COOLFluiD_RADIATION/plugins/RadiativeTransfer/testcases/HuygensDLR/huygens_HSNB_MC_fresh.CFcase >& huygens_HSNB_MC_fresh.CFcase.txt

echo "Running HUYGENS with PARADE: huygens_DLR_PARADE_fresh.CFcase"
mpirun -np 8 ./coolfluid-solver --scase /data2/underbackup/lani/BENJAMIN/TEST_RAD/COOLFluiD_RADIATION/plugins/RadiativeTransfer/testcases/HuygensDLR/huygens_DLR_PARADE_fresh.CFcase >& huygens_DLR_PARADE_fresh.CFcase.txt 

killall -9 parade

####### Radiative properties reused

echo "Running FIREII with HSNB:    fire_viscous_HSNB_MC_reuse_limrange.CFcase"
mpirun -np 8 ./coolfluid-solver --scase /data2/underbackup/lani/BENJAMIN/TEST_RAD/COOLFluiD_RADIATION/plugins/RadiativeTransfer/testcases/FireIIDLR/fire_viscous_HSNB_MC_reuse_limrange.CFcase >& fire_viscous_HSNB_MC_reuse_limrange.CFcase.txt

echo "Running FIREII with PARADE:  fire_viscous_DLR_PARADE_reuse.CFcase"
mpirun -np 8 ./coolfluid-solver --scase /data2/underbackup/lani/BENJAMIN/TEST_RAD/COOLFluiD_RADIATION/plugins/RadiativeTransfer/testcases/FireIIDLR/fire_viscous_DLR_PARADE_reuse.CFcase >& fire_viscous_DLR_PARADE_reuse.CFcase.txt

killall -9 parade

echo "Running HUYGENS with HSNB:   huygens_HSNB_MC_reuse.CFcase"
mpirun -np 8 ./coolfluid-solver --scase /data2/underbackup/lani/BENJAMIN/TEST_RAD/COOLFluiD_RADIATION/plugins/RadiativeTransfer/testcases/HuygensDLR/huygens_HSNB_MC_reuse.CFcase >& huygens_HSNB_MC_reuse.CFcase.txt

echo "Running HUYGENS with PARADE: huygens_DLR_PARADE_reuse.CFcase"
mpirun -np 8 ./coolfluid-solver --scase /data2/underbackup/lani/BENJAMIN/TEST_RAD/COOLFluiD_RADIATION/plugins/RadiativeTransfer/testcases/HuygensDLR/huygens_DLR_PARADE_reuse.CFcase >& huygens_DLR_PARADE_reuse.CFcase.txt 

killall -9 parade



