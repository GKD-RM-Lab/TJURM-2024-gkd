#include "threads/pipeline.h"
#include <unistd.h>
#include <iostream>
// #include <openrm/cudatools.h>
using namespace rm;
// using namespace nvinfer1;
// using namespace nvonnxparser;

//相机驱动
#include "HIKdriver.hpp"

//rmyolo
#include "rmyolov7_inference.h"

//timer
#include "timer.hpp"


void Pipeline::detector_baseline_thread(
    std::mutex& mutex_in, bool& flag_in, std::shared_ptr<rm::Frame>& frame_in, 
    std::mutex& mutex_out, bool& flag_out, std::shared_ptr<rm::Frame>& frame_out
) {
    auto param = Param::get_instance();
    auto garage = Garage::get_instance();

    std::string yolo_type = (*param)["Model"]["YoloArmor"]["Type"];

    int    infer_width       = (*param)["Model"]["YoloArmor"][yolo_type]["InferWidth"];
    int    infer_height      = (*param)["Model"]["YoloArmor"][yolo_type]["InferHeight"];
    int    class_num         = (*param)["Model"]["YoloArmor"][yolo_type]["ClassNum"];
    int    locate_num        = (*param)["Model"]["YoloArmor"][yolo_type]["LocateNum"];
    int    color_num         = (*param)["Model"]["YoloArmor"][yolo_type]["ColorNum"];
    int    bboxes_num        = (*param)["Model"]["YoloArmor"][yolo_type]["BboxesNum"];
    double confidence_thresh = (*param)["Model"]["YoloArmor"][yolo_type]["ConfThresh"];
    double nms_thresh        = (*param)["Model"]["YoloArmor"][yolo_type]["NMSThresh"];

    size_t yolo_struct_size = sizeof(float) * static_cast<size_t>(locate_num + 1 + color_num + class_num);
    std::mutex mutex;
    TimePoint tp0, tp1, tp2;
    cv::Mat inputImage, label_image;
    /*推理模型*/
    yolo_kpt model;
    std::vector<yolo_kpt::Object> result;
    /*帧率统计*/
    Timer timer, timer1, timer2, timer3;

    while (true) {
        // if (!Data::armor_mode) {
        //     std::unique_lock<std::mutex> lock(mutex);
        //     armor_cv_.wait(lock, [this]{return Data::armor_mode;});
        // }

        tp0 = getTime();
        // while(!flag_in) {
        //     if (getDoubleOfS(tp0, getTime()) > 10.0 && Data::timeout_flag) {
        //         rm::message("Detector timeout", rm::MSG_ERROR);
        //         exit(-1);
        //     }
        // }
        
        // std::unique_lock<std::mutex> lock_in(mutex_in);
        // std::shared_ptr<rm::Frame> frame = frame_in;
        // flag_in = false;
        // lock_in.unlock();


        tp1 = getTime();

        // detectOutput(
        //     armor_output_host_buffer_,
        //     armor_output_device_buffer_,
        //     &detect_stream_,
        //     yolo_struct_size,
        //     bboxes_num
        // );
        
        // if (yolo_type == "V5") {
        //     frame->yolo_list = yoloArmorNMS_V5(
        //         armor_output_host_buffer_,
        //         bboxes_num,
        //         class_num,
        //         confidence_thresh,
        //         nms_thresh,
        //         frame->width,
        //         frame->height,
        //         infer_width,
        //         infer_height
        //     );
        // } else if (yolo_type == "FP") {
        //     frame->yolo_list = yoloArmorNMS_FP(
        //         armor_output_host_buffer_,
        //         bboxes_num,
        //         class_num,
        //         confidence_thresh,
        //         nms_thresh,
        //         frame->width,
        //         frame->height,
        //         infer_width,
        //         infer_height
        //     );
        // } else if (yolo_type == "FPX") {
        //     frame->yolo_list = yoloArmorNMS_FPX(
        //         armor_output_host_buffer_,
        //         bboxes_num,
        //         class_num,
        //         confidence_thresh,
        //         nms_thresh,
        //         frame->width,
        //         frame->height,
        //         infer_width,
        //         infer_height
        //     );
        // } else {
        //     rm::message("Invalid yolo type", rm::MSG_ERROR);
        //     exit(-1);
        // }
        
        // if (frame->yolo_list.empty()) {
        //     if (Data::image_flag) imshow(frame);
        //     continue;
        // } 

        // tp2 = getTime();
        // if (Data::pipeline_delay_flag) rm::message("detect time", getDoubleOfS(tp1, tp2) * 1000);

        //读取视频帧
        HIKframemtx.lock();
        HIKimage.copyTo(inputImage);
        HIKframemtx.unlock();
        if(inputImage.empty()) continue;

        /*------识别------*/
        timer1.begin();
        //推理
        result = model.work(inputImage);
        //角点预处理
        model.pnp_kpt_preprocess(result);
        timer1.end();

        /*------可视化------*/
        timer2.begin();
        //输出识别信息&绘图(可视化)
        if(false)
        {
            inputImage.copyTo(label_image);
            label_image = model.visual_label(label_image, result);

            //imshow
            cv::imshow("cam", label_image);
            if(cv::waitKey(1)=='q') break;
        }
        timer2.end();
        
        /*---把识别数据同步到TJU框架的frame中---*/
        timer3.begin();
        rm::Frame frame;        
        std::vector<YoloRect> yolo_retult;  //从result中提取出的yolo结果
        
        for(auto&result_single : result)
        {
            YoloRect yolo_result_single;    //单个装甲板
            //灯条关键点
            if(result_single.kpt.size()<4){
                yolo_result_single.four_points = result_single.kpt; 
            }else{
                //纠正点顺序
                yolo_result_single.four_points.push_back(result_single.kpt[2]);
                yolo_result_single.four_points.push_back(result_single.kpt[1]);
                yolo_result_single.four_points.push_back(result_single.kpt[3]);
                yolo_result_single.four_points.push_back(result_single.kpt[0]);
            }
            //yolo判定框
            yolo_result_single.box = cv::Rect(
                cvRound(result_single.rect.x), 
                cvRound(result_single.rect.y),
                cvRound(result_single.rect.width), 
                cvRound(result_single.rect.height)
            );
            //至信度
            yolo_result_single.confidence = result_single.prob;
            //ID索引转换
            //"B1", "B2", "B3", "B4", "B5", "BO", "BS", "R1", "R2", "R3", "R4", "R5", "RO", "RS"
            //颜色（先假设红0蓝2）,类型（按照兵种编号） TODO 确认
            if(result_single.label < 7){
                yolo_result_single.color_id = rm::ARMOR_COLOR_BLUE;
                yolo_result_single.class_id = result_single.label;
            }else if(result_single.label >= 7){
                yolo_result_single.color_id = rm::ARMOR_COLOR_RED;
                yolo_result_single.class_id = result_single.label - 7;
            }
            
            frame.yolo_list.push_back(yolo_result_single);
            
        }
        
        
        /*同步图片信息到frame中*/
        frame.height = inputImage.rows;    //高
        frame.width = inputImage.cols;     //宽
        frame.camera_id = 0;               //相机id
        if (!frame.image) {
            frame.image = std::make_shared<cv::Mat>();
        }
        inputImage.copyTo(*frame.image); //图像
        frame.time_point = std::chrono::high_resolution_clock::now();  //当前时间
        timer3.end();

        /*计算帧率*/
        timer.end();
        if(true)
        {
            printf("yolo fps = %f\n", 1000.0 / timer.read());
            printf("detector time = %f\n", timer1.read());
            printf("visualize time = %f\n", timer2.read());
            printf("update time = %f\n", timer3.read());
            printf("------------------------------------------");
        }
        timer.begin();


        std::unique_lock<std::mutex> lock_out(mutex_out);
        frame_out = std::make_shared<rm::Frame>(frame);
        flag_out = true;
        lock_out.unlock();
        tracker_in_cv_.notify_one();
    }
}