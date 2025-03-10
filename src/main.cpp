#include "data_manager/base.h"
#include "data_manager/param.h"
#include "threads/pipeline.h"
#include "threads/control.h"
#include "garage/garage.h"

#include <condition_variable>
#include <mutex>
#include <thread>
#include <iostream>
#include <chrono>
#include <unistd.h>


std::mutex hang_up_mutex;
std::condition_variable hang_up_cv;

int main(int argc, char** argv) {
    auto param = Param::get_instance();
    auto pipeline = Pipeline::get_instance();
    auto garage = Garage::get_instance();
    auto control = Control::get_instance();

    int option;
    while ((option = getopt(argc, argv, "hs")) != -1) {
        switch (option) {
            case 's':
                Data::imshow_flag = true;
                break;
            case 'h':
                std::cout << "Usage: " << argv[0] << " [-h] [-s] " << std::endl;
                break;
        }
    }
    
    while(true) if(init_camera()) break;

    rm::message_init("autoaim");
    init_debug();
    init_attack();
    //DEBUG 暂时关闭serial
    // if (Data::serial_flag) init_serial();
    //DEBUG 暂时关闭control
    // control->autoaim();

    //3v3没能量机关，直接baseline
    pipeline->autoaim_baseline();

    // #if defined(TJURM_INFANTRY) || defined(TJURM_BALANCE)
    // pipeline->autoaim_combine();
    // std::cout << "set INFANTRY" << std::endl;  
    // #endif

    // #if defined(TJURM_SENTRY) || defined(TJURM_DRONSE) || defined(TJURM_HERO)
    // pipeline->autoaim_baseline();
    // std::cout << "set SENTRY" << std::endl;  
    // #endif

    while(Data::manu_fire) {
        std::cin.get();
        Data::auto_fire = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Data::auto_fire = false;
    }

    rm::message("Main thread hang up!", rm::MSG_OK);
    std::unique_lock<std::mutex> lock(hang_up_mutex);
    hang_up_cv.wait(lock);
    std::cout << "end" << std::endl;
    return 0;
}