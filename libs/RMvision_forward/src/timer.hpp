#include <chrono>
#include <iostream>

class Timer {
public:
    // 构造函数
    Timer() : start_time(), end_time(), running(false) {}

    // 开始计时
    void begin() {
        start_time = std::chrono::high_resolution_clock::now();
        running = true;
    }

    // 结束计时
    void end() {
        if (running) {
            end_time = std::chrono::high_resolution_clock::now();
            running = false;
        } else {
            std::cerr << "Error: Timer is not running. Call begin() first.\n";
        }
    }

    // 读取时间差，单位为毫秒，精度到微秒
    double read() const {
        if (running) {
            std::cerr << "Warning: Timer is still running. Call end() before read().\n";
        }
        auto duration = std::chrono::duration<double, std::milli>(end_time - start_time);
        return duration.count();
    }

private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
    bool running;
};