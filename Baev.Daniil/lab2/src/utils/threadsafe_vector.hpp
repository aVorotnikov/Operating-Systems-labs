#pragma once

#include <vector>
#include <mutex>

template <typename T>
class Vec
{
public:
    void push(const T& val){
        _m.lock();
        _v.push_back(val);
        _m.unlock();
    }

    T back(){
        _m.lock();
        auto data = _v.back();
        _m.unlock();
        return data;
    }

    void pop(){
        _m.lock();
        _v.pop_back();
        _m.unlock();
    }

    size_t len(void){
        _m.lock();
        size_t size = _v.size();
        _m.unlock();
        return size;
    }

    T& operator[](std::size_t idx){
        return _v[idx];
    }
private:
    std::vector<T> _v;
    mutable std::mutex _m;
};