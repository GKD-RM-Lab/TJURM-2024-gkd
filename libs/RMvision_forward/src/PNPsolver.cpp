#include "PNPsolver.hpp"

//pnp求解全部装甲板
int pnp_solver::calculate_all(std::vector<yolo_kpt::Object> &armors)
{
    for(size_t i=0; i < armors.size(); i++){
        calculate_single(armors[i]);
    }
    return 0;
}

//pnp求解单个装甲板
int pnp_solver::calculate_single(yolo_kpt::Object &armor)
{
    if(armor.pnp_is_calculated == -1)
        return -1;

    std::vector<cv::Point3f> object_4Points;
    std::vector<std::vector<cv::Point3f>> object_3Points;

    //区分大小装甲板
    if(label2string(armor.label) == "B1" || label2string(armor.label) == "R1")
    {
        object_4Points = object_4Points_l;
        object_3Points = object_3Points_l;
        // std::cout << "armor-large detected" << std::endl;

    }else{
        object_4Points = object_4Points_s;
        object_3Points = object_3Points_s;
        // std::cout << "armor-small detected" << std::endl;
    }


    try {
        if (armor.kpt.size() == 4)
        {
            cv::solvePnP(object_4Points, armor.kpt, cameraMatrix, distCoeffs,
                         armor.pnp_rvec, armor.pnp_tvec);
            armor.pnp_is_calculated = 1;
            return 0;
        }
        else if (armor.kpt.size() == 3)
        {
            cv::solvePnP(object_3Points[armor.kpt_lost_index], armor.kpt,
                         cameraMatrix, distCoeffs, armor.pnp_rvec, armor.pnp_tvec,
                         false, cv::SOLVEPNP_SQPNP);
            armor.pnp_is_calculated = 1;
            return 0;
        }
    }
    catch (const cv::Exception &e)
    {
        std::cerr << "solvePnP encountered an error: " << e.what() << std::endl;
        armor.pnp_is_calculated = -1;
        return -1;
    }
    
    return 0;
}

pnp_solver::pnp_solver(const std::string& filename)
{
    //装甲板参数(小装甲板)
    float armor_width_s = params.armor_small_w;      //宽
    float light_height_s = params.armor_small_h;     //高

    //装甲板参数(大装甲板)
    float armor_width_l = params.armor_large_w;      //宽
    float light_height_l = params.armor_large_h;     //高

    //初始化物体坐标系
    std::vector<cv::Point3f> object_all_s = {
        {armor_width_s/2,             light_height_s/2,              0},       //0-左上
        {armor_width_s/2,             -light_height_s/2,             0},       //1-左下
        {-armor_width_s/2,            -light_height_s/2,             0},       //2-右下
        {-armor_width_s/2,            light_height_s/2,              0}        //3-右上
    };

    std::vector<cv::Point3f> object_all_l = {
        {armor_width_l/2,             light_height_l/2,              0},       //0-左上
        {armor_width_l/2,             -light_height_l/2,             0},       //1-左下
        {-armor_width_l/2,            -light_height_l/2,             0},       //2-右下
        {-armor_width_l/2,            light_height_l/2,              0}        //3-右上
    };


    object_4Points_s = object_all_s;
    object_4Points_l = object_all_l;
    for(int i=0; i<4; i++)
    {
        object_3Points_s.push_back(object_all_s);
        object_3Points_s[i].erase(object_3Points_s[i].begin() + i);
        object_3Points_l.push_back(object_all_l);
        object_3Points_l[i].erase(object_3Points_l[i].begin() + i);
    }
    //debug
    std::cout << object_all_l << std::endl;
    std::cout << object_4Points_s << std::endl;
    std::cout << object_3Points_s[0] << std::endl;
    std::cout << object_3Points_s[1] << std::endl;

    readCameraParametersFromYaml(filename);

}

// 从YAML文件读取cameraMatrix和distCoeffs
void pnp_solver::readCameraParametersFromYaml(const std::string& filename) {

    cv::FileStorage fs(filename, cv::FileStorage::READ);
    fs["camera_matrix"] >> cameraMatrix;
    fs["distortion_coefficients"] >> distCoeffs;
}


std::string pnp_solver::label2string(int num) {
    std::vector<std::string> class_names = {
        "B1", "B2", "B3", "B4", "B5", "BO", "BS", "R1", "R2", "R3", "R4", "R5", "RO", "RS"
    };
    return class_names[num];
}

pnp_solver::~pnp_solver()
{

}