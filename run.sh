# please run at the root folder -- MTR/
set -e
cmake -B build
cmake --build build
./build/mtr obj/diablo3_pose.obj output