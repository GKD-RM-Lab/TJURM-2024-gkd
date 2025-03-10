#include "parameter_loader.hpp"

int yaml_write(const std::string& filepath, const parameter_loader_t& params);
int yaml_load(const std::string& filepath, parameter_loader_t& params);
int para_load(std::string filepath);

parameter_loader_t params;


int para_load(std::string filepath)
{
    // debug 写入以生成yaml文件
    // yaml_write(filepath, params);
    // return 0;
    yaml_load(filepath, params);
    // std::cout << params.armor_small_h << std::endl;
}





// 保存参数到 YAML 文件
int yaml_write(const std::string& filepath, const parameter_loader_t& params) {
    cv::FileStorage fs(filepath, cv::FileStorage::WRITE);
    if (!fs.isOpened()) {
        std::cerr << "错误：无法打开文件 " << filepath << " 用于写入！" << std::endl;
        return -1;
    }

    /*推理参数*/
    fs << "model_path_xml" << params.model_path_xml;
    fs << "model_path_bin" << params.model_path_bin;
    fs << "conf_threshold" << params.conf_threshold;
    fs << "rect_cut" << params.rect_cut;

    /*标定参数*/
    fs << "boardSize_h" << params.boardSize_h;
    fs << "boardSize_w" << params.boardSize_w;
    fs << "squareSize" << params.squareSize;
    fs << "img_count" << params.img_count;
    fs << "sample_period" << params.sample_period;
    fs << "calib_yaml_path" << params.calib_yaml_path;

    /*相机参数*/
    fs << "cam_gain" << params.cam_gain;
    fs << "cam_exptime" << params.cam_exptime;
    fs << "framerate" << params.framerate;

    /*其他*/
    fs << "imshow_en" << params.imshow_en;
    fs << "debug_info" << params.debug_info;
    fs << "armor_small_h" << params.armor_small_h;
    fs << "armor_small_w" << params.armor_small_w;
    fs << "armor_large_h" << params.armor_large_h;
    fs << "armor_large_w" << params.armor_large_w;

    fs.release();
    std::cout << "参数已保存到 " << filepath << std::endl;
    return 0;
}

// 从 YAML 文件加载参数
int yaml_load(const std::string& filepath, parameter_loader_t& params) {
    cv::FileStorage fs(filepath, cv::FileStorage::READ);
    if (!fs.isOpened()) {
        std::cerr << "错误：无法打开文件 " << filepath << " 用于读取！" << std::endl;
        return -1;
    }

    /*推理参数*/
    fs["model_path_xml"] >> params.model_path_xml;
    fs["model_path_bin"] >> params.model_path_bin;
    fs["conf_threshold"] >> params.conf_threshold;
    fs["rect_cut"] >> params.rect_cut;

    /*标定参数*/
    fs["boardSize_h"] >> params.boardSize_h;
    fs["boardSize_w"] >> params.boardSize_w;
    fs["squareSize"] >> params.squareSize;
    fs["img_count"] >> params.img_count;
    fs["sample_period"] >> params.sample_period;
    fs["calib_yaml_path"] >> params.calib_yaml_path;

    /*相机参数*/
    fs["cam_gain"] >> params.cam_gain;
    fs["cam_exptime"] >> params.cam_exptime;
    fs["framerate"] >> params.framerate;

    /*其他*/
    fs["imshow_en"] >> params.imshow_en;
    fs["debug_info"] >> params.debug_info;
    fs["armor_small_h"] >> params.armor_small_h;
    fs["armor_small_w"] >> params.armor_small_w;
    fs["armor_large_h"] >> params.armor_large_h;
    fs["armor_large_w"] >> params.armor_large_w;

    fs.release();
    std::cout << "参数已从 " << filepath << " 加载" << std::endl;
    return 0;
}
