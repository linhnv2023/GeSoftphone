#pragma once

#include <string>
#include <cstdint>

enum CallType {
    CALL_TYPE_OUTGOING = 0,
    CALL_TYPE_INCOMING = 1,
    CALL_TYPE_MISSED = 2,
    CALL_TYPE_AUTO_ANSWERED = 3
};

class Call
{
public:
    int id = 0;
    std::string callId;
    std::string number;
    std::string name;
    int type = CALL_TYPE_OUTGOING;
    int64_t time = 0;
    int duration = 0;
    std::string info;
};
