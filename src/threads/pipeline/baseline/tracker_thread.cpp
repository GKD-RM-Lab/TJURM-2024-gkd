#include "threads/pipeline.h"
#include "threads/control.h"

#include "timer.hpp"

using namespace rm;

void Pipeline::tracker_baseline_thread(
    std::mutex& mutex_in, bool& flag_in, std::shared_ptr<rm::Frame>& frame_in
) {

    //性能估计
    Timer timer, timer1, timer2;
    
    auto garage = Garage::get_instance();
    auto param = Param::get_instance();
    auto control = Control::get_instance();
    
    // 获取装甲板长宽
    float big_width    = (*param)["Points"]["PnP"]["Red"]["BigArmor"]["Width"];
    float big_height   = (*param)["Points"]["PnP"]["Red"]["BigArmor"]["Height"];
    float small_width  = (*param)["Points"]["PnP"]["Red"]["SmallArmor"]["Width"];
    float small_height = (*param)["Points"]["PnP"]["Red"]["SmallArmor"]["Height"];
    
    std::string small_path = (*param)["Debug"]["SmallDecal"];
    std::string big_path   = (*param)["Debug"]["BigDecal"];
    
    init_pointer();
    init_locater();
    init_updater();
    
    if(Data::reprojection_flag) {
        initReprojection(small_width, small_height, big_width, big_height, small_path, big_path);
    }
    
    rm::CycleQueue<double> delay_list(100);
    TimePoint tp0, tp1, tp2;
    
    std::mutex mutex;
    while (true) {
        if (!Data::armor_mode) {
            std::unique_lock<std::mutex> lock(mutex);
            armor_cv_.wait(lock, [this]{return Data::armor_mode;});
        }

        std::unique_lock<std::mutex> lock_in(mutex_in);
        tracker_in_cv_.wait(lock_in, [&flag_in]{return flag_in;});

        std::shared_ptr<rm::Frame> frame = frame_in;
        flag_in = false;
        lock_in.unlock();

        //deug 假设云台全是0位
        frame->yaw = 0;
        frame->pitch = 0;
        frame->roll = 0;
        
        timer1.begin();
        tp1 = getTime();
        bool track_flag = true;
        if (track_flag) track_flag = pointer(frame);    //ok
        if (track_flag) track_flag = locater(frame);    //ok
        if (track_flag) track_flag = updater(frame);
        tp2 = getTime();
        timer1.end();

        /*debug pnp data*/
        if(frame->target_list.size()>0 && false){
            std::cout << "pose" << frame->target_list[0].pose_world << std::endl;
            std::cout << "armor_yaw_world" << frame->target_list[0].armor_yaw_world << std::endl;        
        }

        if (Data::pipeline_delay_flag) rm::message("tracker time", getDoubleOfS(tp1, tp2) * 1000);
        if (track_flag) delay_list.push(getDoubleOfS(tp0, tp2));

        tp0 = tp2;
        double fps = 1.0 / delay_list.getAvg();
        rm::message("fps", fps);
        
        if (Data::image_flag){
            if (Data::ui_flag) UI(frame);
            imshow(frame);
        }

        /*debug*/
        timer.end();
        if(false)
        {
            printf("---------------------");
            printf("tracker fps = %f\n", 1000 / timer.read());
            printf("calculate time = %f ms \n", timer1.read());
            if(frame->target_list.size()>0){
                std::cout << "target[0]" << frame->target_list[0].pose_world << std::endl;
            }
        }
        timer.begin();

    }
}