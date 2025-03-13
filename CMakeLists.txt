cmake_minimum_required(VERSION 3.12)
project(TJURM-2024)

# 设置编译指令
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE DEBUG)
add_compile_options(-g)

# 目标兵种保持要选的即可
add_definitions(-DTJURM_INFANTRY)

# ============= 必须保留的原有配置 =============
# OpenRM核心库配置（必须保留！）
find_package(OpenRM REQUIRED)
include_directories(${OpenRM_INCLUDE_DIRS})

# ============= 新增视觉前端配置 =============
# OpenVINO配置
set(OpenVINODeveloperPackage_DIR "/usr/lib/cmake/openvino2024.6.0")
find_package(OpenVINO REQUIRED)
if (NOT DEFINED openvino_VERSION)
    message(WARNING "OpenVINO was found, but version could not be determined.")
else()
    message(STATUS "Using OpenVINO version ${openvino_VERSION}")
endif()

# 海康相机SDK配置
set(VISION_FORWARD_DIR ${CMAKE_SOURCE_DIR}/libs/RMvision_forward)
set(CAMERA_LIB_DIR "${VISION_FORWARD_DIR}/lib/64")
include_directories(
    ${VISION_FORWARD_DIR}/lib/include
    ${VISION_FORWARD_DIR}/src  # 包含前端的头文件
)
link_directories(${CAMERA_LIB_DIR})

# ============= 其他依赖保持原样 =============
# OpenCV
find_package(OpenCV 4.5.4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Eigen
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Ceres
find_package(Ceres REQUIRED)
include_directories(${CERES_INCLUDE_DIRS})

# 项目头文件
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/libs/json/single_include/nlohmann
)

# ============= 收集前端源文件 =============
file(GLOB_RECURSE VISION_SOURCE_FILES
    "${VISION_FORWARD_DIR}/src/*.cpp"
    "${VISION_FORWARD_DIR}/src/*.hpp"
    "${CMAKE_SOURCE_DIR}/src/send_control/*.cpp"
)

# 构建子目录（原有结构）
add_subdirectory(src/data_manager)
add_subdirectory(src/garage)
add_subdirectory(src/threads)

# 可执行文件
add_executable(TJURM-2024 
    src/main.cpp 
    ${VISION_SOURCE_FILES}
)

# ============= 链接库顺序调整 =============
target_link_libraries(TJURM-2024
    data_manager
    garage
    threads
    # 原有依赖
    ${OpenRM_LIBRARIES}  # 必须保留！
    # OpenCV
    ${OpenCV_LIBS}
    # OpenVINO
    openvino::runtime
    # 海康相机
    MvCameraControl
    MVGigEVisionSDK
    FormatConversion
    GCBase_gcc421_v3_0
    GenApi_gcc421_v3_0
    log4cpp_gcc421_v3_0
    Log_gcc421_v3_0
    MathParser_gcc421_v3_0
    MediaProcess
    MVRender
    MvUsb3vTL
    NodeMapData_gcc421_v3_0
    XmlParser_gcc421_v3_0
    # 系统库
    X11
    pthread
    dl
)

# 安装配置保持原有
install(TARGETS TJURM-2024
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)