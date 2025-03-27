#!/bin/bash

blue="\033[1;34m"
yellow="\033[1;33m"
reset="\033[0m"

include_count=$(find include  -type f \( -name "*.cpp" -o -name "*.h" \) -exec cat {} \; | wc -l)
src_count=$(find src  -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.txt" \) -exec cat {} \; | wc -l)
total=$((include_count + src_count))

if [ ! -d "data/debug" ]; then
    mkdir data/debug
    touch data/debug/here_save_debug_images
fi

if [ ! -d "data/video" ]; then
    mkdir data/video
    touch data/video/here_save_video
fi

if [ ! -d "data/speed" ]; then
    mkdir data/speed
    touch data/speed/here_save_shoot_speed
fi

if [ ! -d "/etc/openrm" ]; then 
    mkdir /etc/openrm
    sudo cp -r data/uniconfig/* /etc/openrm/
    sudo chmod -R 777 /etc/openrm
fi

if [ ! -d "config" ]; then 
    ln -s /etc/openrm ./config
fi

if [ ! -d "build" ]; then 
    mkdir build
fi

imshow=0
verbose=0

# 注意：这里 *不再* 把 v 写进 getopts 的选项列表中
while getopts ":rcg:ls" opt; do
    case $opt in
        r)
            echo -e "${yellow}<--- delete 'build' --->\n${reset}"
            sudo rm -rf build
            mkdir build
            ;;
        c)
            sudo cp -r data/uniconfig/* /etc/openrm/
            sudo chmod -R 777 /etc/openrm
            exit 0
            ;;
        g)
            git_message=$OPTARG
            echo -e "${yellow}\n<--- Git $git_message --->${reset}"
            git pull
            git add -A
            git commit -m "$git_message"
            git push
            exit 0
            ;;
        l)
            cd ../OpenRM
            sudo ./run.sh
            cd ../TJURM-2024
            exit 0
            ;;
        s)
            imshow=1
            ;;
        \?)
            echo -e "${red}\n--- Unavailable param: -$OPTARG ---\n${reset}"
            ;;
        :)
            echo -e "${red}\n--- param -$OPTARG need a value ---\n${reset}"
            ;;
    esac
done

# 将所有已处理过的选项移除，剩下的即是非选项参数
shift $((OPTIND - 1))

echo -e "${yellow}<--- Start CMake --->${reset}"
cd build
cmake ..

echo -e "${yellow}\n<--- Start Make --->${reset}"
max_threads=$(cat /proc/cpuinfo | grep "processor" | wc -l)
make -j "$max_threads"

echo -e "${yellow}\n<--- Total Lines --->${reset}"
echo -e "${blue}        $total${reset}"

# ========== 新增：模型文件安装 ==========
VISION_FORWARD_DIR="../libs/RMvision_forward"
MODEL_DIR="/etc/openrm/models"
if [ ! -d "${MODEL_DIR}" ]; then
    sudo mkdir -p ${MODEL_DIR}
    sudo chmod -R 777 /etc/openrm
fi
sudo cp -r ${VISION_FORWARD_DIR}/models/* ${MODEL_DIR}/

# ========== 新增：驱动库安装 ==========
DRIVER_DIR="/etc/openrm/cam_driver"
if [ ! -d "${DRIVER_DIR}" ]; then
    sudo mkdir -p ${DRIVER_DIR}
    sudo chmod -R 777 /etc/openrm
fi
sudo cp -r ${VISION_FORWARD_DIR}/lib/* ${DRIVER_DIR}/

# ========== 新增：驱动库软链接 ==========
LIB_PATH="${DRIVER_DIR}/64"
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${LIB_PATH}
# echo "LD_LIBRARY_PATH updated to: $LD_LIBRARY_PATH"

# ========== 新增：前端配置文件安装 ==========
FORWARD_CONFIG_DIR="/etc/openrm/forward_config"
if [ ! -d "${FORWARD_CONFIG_DIR}" ]; then
    sudo mkdir -p "${FORWARD_CONFIG_DIR}"
    sudo chmod -R 777 /etc/openrm
fi

sudo rm -rf "${FORWARD_CONFIG_DIR}"/*

# 第一个非选项参数是兵种
TARGET="$1"
case "$TARGET" in
    hero|infantry|sentry_l|sentry_r)
        echo "Copy ${TARGET} config to ${FORWARD_CONFIG_DIR} ..."
        sudo cp -r "${VISION_FORWARD_DIR}/config/${TARGET}/"* "${FORWARD_CONFIG_DIR}/"
        ;;
    *)
        echo "无效或未指定兵种参数，跳过拷贝前端配置文件"
        exit 1
        ;;
esac

# 再 shift 掉兵种参数，然后看下一个是否是 -v
shift
if [ "$1" = "-v" ]; then
    verbose=1
fi

sudo rm /usr/local/bin/TJURM-2024
sudo cp TJURM-2024 /usr/local/bin/
sudo pkill TJURM-2024
sudo chmod 777 /dev/tty*

# 根据是否选择 -v / -s 决定调用
if [ $verbose = 1 ]; then
    TJURM-2024 -v
elif [ $imshow = 1 ]; then
    TJURM-2024 -s
else
    TJURM-2024
fi

/etc/openrm/guard.sh

echo -e "${yellow}<----- OVER ----->${reset}"
