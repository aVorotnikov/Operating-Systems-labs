#pragma once
#include "goat_info.h"
#include <string>

struct Message
{
    enum MESSAGE_TYPE
    {
        MT_END = 0,
        MT_KEEP_WAITING,
        MT_THROW_REQUEST,
        MT_THROW_RESPONSE,
        MT_ROUND_RESULT,
    };

    MESSAGE_TYPE messageType;
    GoatInfo goatInfo;
    Message() : messageType(MT_END), goatInfo(){};
    Message(MESSAGE_TYPE type, const GoatInfo &info) : messageType(type), goatInfo(info){};
};

struct GUIMessage
{
    enum GUI_MESSAGE_TYPE
    {
        GMT_END,
        GMT_UPDATE_TABLE,
        GMT_SHOW_LOG,
        GMT_NEW_ROUND,
        GMT_GAME_OVER,
    };

    GUI_MESSAGE_TYPE messageType;
    std::string logText;

    GUIMessage(GUI_MESSAGE_TYPE type) : messageType(type){};
    GUIMessage(std::string logText) : messageType(GUI_MESSAGE_TYPE::GMT_SHOW_LOG), logText(logText){};
};

inline bool operator==(const GUIMessage &a, const GUIMessage &b)
{
    return a.messageType == b.messageType;
}

struct HostMessage
{
    enum HOST_MESSAGE_TYPE
    {
        HMT_END,
        HMT_START_GAME,
        HMT_GUI_INPUT_END,
    };

    HOST_MESSAGE_TYPE messageType;

    HostMessage(HOST_MESSAGE_TYPE type) : messageType(type){};
};