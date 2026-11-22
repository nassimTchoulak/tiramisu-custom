export TIRAMISU_ROOT_DIR=/home/nass/Desktop/isl_tiramisu/tiramisu/
export TIRAMISU_ROOT=/home/nass/Desktop/isl_tiramisu/tiramisu/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/nass/Desktop/isl_tiramisu/tiramisu/3rdParty/Halide/lib/
cd /home/nass/Desktop/isl_tiramisu/tiramisu/tutorials/tutorial_autoscheduler/general_scheduling_simple
rm -rf build && mkdir build
cd build
cmake ..
make
../generator > ../log_exec.txt