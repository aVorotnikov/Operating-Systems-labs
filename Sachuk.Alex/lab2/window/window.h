#pragma once

#include <atomic>
#include <string>
#include <functional>

using read_callback_t = void(*)(Message msg);
using write_callback_t = void(*)(Message *msg);
using controll_callback_t = bool(*)(Message msg);

class Window {
private:
    std::atomic<bool> isConnected = false;
    std::string windowName;

    read_callback_t winReadCallback;
    write_callback_t winWriteCallback;
    controll_callback_t controlCallback;
public:
    Window(std::string name, read_callback_t readCallback, write_callback_t writeCallback, controll_callback_t controllCallback) : 
        windowName(name), winReadCallback(readCallback), winWriteCallback(writeCallback), controlCallback(controllCallback) {};

    void setConnected(bool isConnected) { this->isConnected.store(isConnected); }
    int run();

    ~Window() = default;
};