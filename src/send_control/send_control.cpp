#include "send_control/send_control.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>

#include "parameter_loader.hpp"

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
    uint8_t header;
    float yaw_set;
    float pitch_set;
    bool fire;

    ROBOT_MODE mode;
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

void send_control(double yaw_set, double pitch_set, bool fire)
{
    Vison_control pkg{};

    if(params.sentry_head_type == 1){
        pkg.header = 0x6b;  //哨兵左头 自瞄和电控在不同nuc
    }else{
        pkg.header = 0x6A;
    }
    pkg.yaw_set = yaw_set;
    pkg.pitch_set = pitch_set;
    pkg.fire = fire;


    //printf("send control\n");
    auto n = sendto(
    sockfd,
    (const char *)(&pkg),
    sizeof(pkg),
    MSG_CONFIRM,
    (const struct sockaddr *)&addr,
    sizeof(addr));
}
