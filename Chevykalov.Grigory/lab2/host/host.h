#ifndef __HOST_H_
#define __HOST_H_

#include <sys/types.h>
#include <atomic>
#include <vector>
#include <mutex>
#include <memory>
#include <cstdlib>
#include <bits/types/siginfo_t.h>

#include "../gui/gui.h"
#include "../connection/connection.h"

constexpr char DEAD[] = "dead";
constexpr char ALIVE[] = "alive";

class Host {
private:
    template <typename T>
    class SingleCont {
    private:
        T _v;
        int _s = 0;
        mutable std::mutex _m;
    public:
	    void Set(const T &val) {
		    _m.lock();
            _v = val;
            _s = 1;
		    _m.unlock();
	    }
	
	    bool Get(T *data) {
		    _m.lock();
		    if (_s == 0) {
			    _m.unlock();
			    return false;
		    }
		    *data = _v;
		    _s = 0;
		    _m.unlock();
		    return true;
	    }

        int len(void) {
            return _s;
        }
    };

    const int _timeInter = 5;
    const int _tMax = 3;
    const int _lMax = 2;
    const int _maxNum = 100;
    const int _aliveInter = 70;
    const int _deadInter = 20;

    SingleCont<GState> _output;
    SingleCont<int> _input;

    std::atomic<pid_t> _clientPid = -1;
    std::atomic<bool> _isTerminated = false;
    std::atomic<int> _time = 0;
    GUI *_gui = nullptr;

    std::unique_ptr<Connection> _conn;
    sem_t *_semRead, *_semWrite;

    static void SignalHandler(int signum, siginfo_t *info, void *ptr);

    static void GUISend(int num) {
        Host::GetInstance()._input.Set(num);
    }
    static bool GUIGet(GState *st) {
        return Host::GetInstance()._output.Get(st);
    }
    static int GUITimer(void) {
        return GetInstance()._time.load();
    }
    static bool GUIIsTerminated(void) {
        return GetInstance()._isTerminated.load();
    }
    static bool GUIIsConnected(void) {
        return GetInstance()._clientPid.load() != -1;
    }
    static bool GUINeedToSend(void) {
        return GetInstance()._input.len() == 0;
    }

    void Work(void);

    bool OpenConnection(void);
    bool GetNum(int *num);
    bool SendState(bool st);
    void CloseConnection(void);

    Host(void);

    Host(const Host&) = delete;
    Host& operator=(const Host&) = delete;
public:

    static Host &GetInstance(void) noexcept {
        static Host instance;
        return instance;
    }

    int Run(void) noexcept;

    void Terminate(void) noexcept;

    ~Host() = default;
};

#endif //!__HOST_H_