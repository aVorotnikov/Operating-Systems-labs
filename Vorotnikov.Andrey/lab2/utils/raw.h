#pragma once

#include "../connections/conn.h"

namespace utils
{

template <typename Type>
bool SendRaw(std::shared_ptr<Connection> connection, const Type t)
{
    return connection->Write(&t, sizeof(Type));
}

template <typename Type>
bool GetRaw(std::shared_ptr<Connection> connection, Type& t)
{
    return connection->Read(&t, sizeof(Type));
}

}
