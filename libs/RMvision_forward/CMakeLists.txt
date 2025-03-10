cmake_minimum_required(VERSION 3.10)
project(cl_yolo)

# 设置 C++ 标准（确保支持 C++11 及以上）
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# 1. 查找并配置 OpenCV
# ------------------------------------------------------------------------------
find_package(OpenCV REQUIRED)
if (NOT OpenCV_FOUND)
    message(FATAL_ERROR "未找到 OpenCV，请先安装或配置 OpenCV！")
endif()
message(STATUS "Using OpenCV version ${OpenCV_VERSION}")

# ------------------------------------------------------------------------------
# 2. 配置 OpenVINO
# ------------------------------------------------------------------------------
# 如有需要，可根据你安装的版本更新路径:
set(OpenVINODeveloperPackage_DIR "/usr/lib/cmake/openvino2024.6.0")
find_package(OpenVINO REQUIRED)
if (NOT DEFINED openvino_VERSION)
    message(WARNING "OpenVINO was found, but version could not be determined.")
else()
    message(STATUS "Using OpenVINO version ${openvino_VERSION}")
endif()

# ------------------------------------------------------------------------------
# 3. 配置海康相机 SDK
# ------------------------------------------------------------------------------
# 假设 64 位库都放在 lib/64 目录下
set(CAMERA_LIB_DIR "${CMAKE_SOURCE_DIR}/lib/64")
link_directories(${CAMERA_LIB_DIR})

# ------------------------------------------------------------------------------
# 4. 包含头文件目录
# ------------------------------------------------------------------------------
# - include/           (你的项目公共头文件)
# - ${CMAKE_SOURCE_DIR}/include (海康SDK头文件所在处)
# - ${OpenCV_INCLUDE_DIRS}
# - ${OpenVINO_INCLUDE_DIRS}   (如果需要手动包含)
include_directories(
    ${CMAKE_SOURCE_DIR}/lib/include
    ${CMAKE_SOURCE_DIR}/lib
    ${OpenCV_INCLUDE_DIRS}
    # ${OpenVINO_INCLUDE_DIRS}  # 通常 find_package(OpenVINO) 已经处理
)

# ------------------------------------------------------------------------------
# 5. 收集源文件
# ------------------------------------------------------------------------------
# 你项目中所有 .cpp（含 HIKdriver.cpp）均在 src/ 目录下
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# ------------------------------------------------------------------------------
# 6. 创建可执行文件
# ------------------------------------------------------------------------------
add_executable(cl_yolo ${SOURCE_FILES})

# ------------------------------------------------------------------------------
# 7. 链接依赖库
# ------------------------------------------------------------------------------
# - OpenCV
# - OpenVINO runtime
# - 海康 SDK (对应 lib/64 下 .so 去掉前缀 lib 和后缀 .so)
# - 以及系统库 X11、pthread 等
target_link_libraries(cl_yolo
    ${OpenCV_LIBS}
    openvino::runtime

    # 海康相机驱动相关库：注意顺序通常无关紧要，但可保持与原示例一致
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

    # 可能还需额外库
    X11
    pthread
)

# ------------------------------------------------------------------------------
# 8. 可选：添加编译器选项
# ------------------------------------------------------------------------------
if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(cl_yolo PRIVATE -Wall)
endif()

# ------------------------------------------------------------------------------
# 9. 可选：OpenVINO 模板插件
# ------------------------------------------------------------------------------
option(ENABLE_TEMPLATE_REGISTRATION "Enables registration of TEMPLATE plugin" OFF)
if(ENABLE_TEMPLATE_REGISTRATION)
    target_compile_definitions(cl_yolo PRIVATE ENABLE_TEMPLATE_REGISTRATION)
endif()
