#include "send_control/send_control.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>

int64_t port_num = 11452;
int sockfd;
sockaddr_in serv_addr;
sockaddr_in addr;

enum ROBOT_MODE
{
    ROBOT_NO_FORCE,
    ROBOT_FINISH_INIT,
    ROBOT_FOLLOW_GIMBAL,
    ROBOT_SEARCH,
    ROBOT_IDLE,
    ROBOT_NOT_FOLLOW
};

struct Vison_control
{
    /*       导航部分       */
    uint8_t header;
    // 线速度 m/s
    float linear_vx;
    float linear_vy;
    // 旋转角速度 rad/s
    float angular;
    // 欧拉角
    float yaw_set;
    float pitch_set;


    ROBOT_MODE mode = ROBOT_MODE::ROBOT_IDLE;
} __attribute__((packed));

void init_send(std::string ip)
{
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(11451);


    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("can't open socket\n");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_num);


    if (bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("can't bind socket fd with port number");
    }
}

void send_control(double yaw_set, double pitch_set)
{
    Vison_control pkg{};

    pkg.header = 0x6A;
    pkg.yaw_set = yaw_set;
    pkg.pitch_set = pitch_set;


    auto n = sendto(
    sockfd,
    (const char *)(&pkg),
    sizeof(pkg),
    MSG_CONFIRM,
    (const struct sockaddr *)&addr,
    sizeof(addr));
}