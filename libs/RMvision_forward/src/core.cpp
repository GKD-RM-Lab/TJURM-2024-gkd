#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>



#include "rmyolov7_inference.h"
#include "HIKdriver.hpp"

#include "timer.hpp"

//cam calib
#include "cam_calibration.hpp"

//pnpsolver
#include "PNPsolver.hpp"

//openvino
#include <openvino/openvino.hpp>

#include <thread>


//loader
#include "parameter_loader.hpp"

void gpu_accel_check();

int main(int argc, char** argv) {

    //载入参数
    para_load("../config/config.yaml");
    // return 0;   //debug

    /*相机读取线程*/
    std::thread cameraThread(HIKcamtask);
    cv::Mat inputImage;

    float total_time;   //debug 每识别周期时间

    /*相机标定*/
    if(argc > 1)
    {
        std::string command_str;
        command_str = argv[1];    
        std::cout << command_str << std::endl;
        if(command_str == "--calibration")
        {
            //相机标定
            std::cout << "into camera calibration ....." << std::endl;
            calibration_main();
            return 0;
        }else{
            std::cout << "usage: " << std::endl;
            std::cout << "--calibration -> to calibrate camera" << std::endl;
        }
    }

    //启用opencl
    cv::ocl::setUseOpenCL(true);
    gpu_accel_check();

    /*推理模型*/
    yolo_kpt model;
    std::vector<yolo_kpt::Object> result;

    /*计时器*/
    Timer timer, timer2;
    timer2.begin();

    /*PNP求解器*/
    pnp_solver pnp("../config/camera_paramets.yaml");
    std::cout << "camera intrinsics is loaded to :" << std::endl;
    std::cout << "cameraMatrix:" << std::endl;
    std::cout << pnp.cameraMatrix << std::endl;
    std::cout << "distCoeffs:" << std::endl;
    std::cout << pnp.distCoeffs << std::endl;

    while(1)
    {   
        //读取视频帧
        HIKframemtx.lock();
        HIKimage.copyTo(inputImage);
        HIKframemtx.unlock();
        if(inputImage.empty()) continue;

        if(params.rect_cut == 1){
            inputImage = rect_cut(inputImage);
        }

        /*识别*/
        //识别图像（前处理+推理+后处理）
        timer.begin();
        result = model.work(inputImage);
        timer.end();
        std::cout << "total time:" << timer.read() << std::endl;
        std::cout << "--------------------" << std::endl;
        
        /*PNP*/
        //角点预处理
        model.pnp_kpt_preprocess(result);

        //pnp求解
        pnp.calculate_all(result);

        //fps
        char text[50];
        std::sprintf(text, "%.2fps, %.2fms", 1000/total_time, total_time);
        cv::putText(inputImage, text, cv::Point(0,30)
            , cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0,255,0), 3);
        
        //输出识别信息&绘图
        inputImage = model.visual_label(inputImage, result);
        if(params.imshow_en == 1){
            cv::imshow("label", inputImage);
        }    
        if(cv::waitKey(1) == 'q') break;

        timer2.end();
        std::cout << "display->" << 1000/timer2.read() << "fps" << std::endl;
        total_time = (float)timer2.read();
        timer2.begin();
    }
    return 0;
}



void gpu_accel_check()
{
    //check opencl
    if (cv::ocl::haveOpenCL()) {
        std::cout << "OpenCL is supported." << std::endl;
    } else {
        std::cout << "OpenCL is not supported." << std::endl;
    }

    //check DNN
    // 获取所有可用的后端
    std::vector<std::pair<cv::dnn::Backend, cv::dnn::Target>> backendsAndTargets = cv::dnn::getAvailableBackends();

    std::cout << "Available DNN Backends and Targets:" << std::endl;
    for (auto &backendAndTarget : backendsAndTargets) {
        cv::dnn::Backend backend = backendAndTarget.first;
        cv::dnn::Target target = backendAndTarget.second;

        // 根据返回的 Backend 枚举类型打印具体名称
        switch (backend) {
            case cv::dnn::DNN_BACKEND_DEFAULT:
                std::cout << "Backend: DNN_BACKEND_DEFAULT, ";
                break;
            case cv::dnn::DNN_BACKEND_HALIDE:
                std::cout << "Backend: DNN_BACKEND_HALIDE, ";
                break;
            case cv::dnn::DNN_BACKEND_INFERENCE_ENGINE:
                std::cout << "Backend: DNN_BACKEND_INFERENCE_ENGINE, ";
                break;
            case cv::dnn::DNN_BACKEND_OPENCV:
                std::cout << "Backend: DNN_BACKEND_OPENCV, ";
                break;
            case cv::dnn::DNN_BACKEND_VKCOM:
                std::cout << "Backend: DNN_BACKEND_VKCOM, ";
                break;
            case cv::dnn::DNN_BACKEND_CUDA:
                std::cout << "Backend: DNN_BACKEND_CUDA, ";
                break;
            default:
                std::cout << "Backend ID: " << backend << ", ";
                break;
        }

        // 根据返回的 Target 枚举类型打印具体名称
        switch (target) {
            case cv::dnn::DNN_TARGET_CPU:
                std::cout << "Target: DNN_TARGET_CPU" << std::endl;
                break;
            case cv::dnn::DNN_TARGET_OPENCL:
                std::cout << "Target: DNN_TARGET_OPENCL" << std::endl;
                break;
            case cv::dnn::DNN_TARGET_OPENCL_FP16:
                std::cout << "Target: DNN_TARGET_OPENCL_FP16" << std::endl;
                break;
            case cv::dnn::DNN_TARGET_CUDA:
                std::cout << "Target: DNN_TARGET_CUDA" << std::endl;
                break;
            case cv::dnn::DNN_TARGET_CUDA_FP16:
                std::cout << "Target: DNN_TARGET_CUDA_FP16" << std::endl;
                break;
            default:
                std::cout << "Target ID: " << target << std::endl;
                break;
        }
    }
    printf("-------------------\n");
}
