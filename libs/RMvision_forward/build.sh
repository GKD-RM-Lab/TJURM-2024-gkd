SCRIPT_DIR=$(cd $(dirname "$0") && pwd)
LIB_PATH="${SCRIPT_DIR}/lib/64"
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${LIB_PATH}
echo "LD_LIBRARY_PATH updated to: $LD_LIBRARY_PATH"

mkdir build
cd build
cmake ..
make -j
# ./cl_yolo --calibration
./cl_yolo