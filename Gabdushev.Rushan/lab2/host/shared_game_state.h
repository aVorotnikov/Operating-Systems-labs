#pragma once
#include "utils/goat_info.h"
#include "connections/conn.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <algorithm>

class SharedGameState
{
private:
    std::mutex goatTableAccess;
    std::vector<GoatInfo> goatTable;
    short nextId = 1;

public:
    std::atomic_ushort wolfNumber;
    std::size_t size()
    {
        std::scoped_lock lock(goatTableAccess);
        return goatTable.size();
    }
    std::vector<GoatInfo> getGoatTable()
    {
        std::scoped_lock lock(goatTableAccess);
        return std::vector<GoatInfo>(goatTable);
    }
    bool getGoatInfo(short id, GoatInfo &goatInfoOut)
    {
        std::scoped_lock lock(goatTableAccess);
        auto item = std::find_if(goatTable.begin(), goatTable.end(), [id](GoatInfo &x)
                                 { return x.id == id; });
        if (item == goatTable.end())
        {
            return false;
        }
        goatInfoOut = *item;
        return true;
    }
    bool removeGoatInfo(short id)
    {
        std::scoped_lock lock(goatTableAccess);
        auto item = std::find_if(goatTable.begin(), goatTable.end(), [id](GoatInfo &x)
                                 { return x.id == id; });
        if (item == goatTable.end())
        {
            return false;
        }
        goatTable.erase(item);
        return true;
    }
    bool updateGoatInfo(const GoatInfo &goatInfo)
    {
        std::scoped_lock lock(goatTableAccess);
        auto id = goatInfo.id;
        auto item = std::find_if(goatTable.begin(), goatTable.end(), [id](GoatInfo &x)
                                 { return x.id == id; });
        if (item == goatTable.end())
        {
            return false;
        }
        *item = goatInfo;
        return true;
    }
    short addGoat()
    {
        std::scoped_lock lock(goatTableAccess);
        goatTable.push_back(GoatInfo(nextId));
        return nextId++;
    }
};