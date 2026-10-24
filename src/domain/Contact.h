#pragma once

#include <string>
#include <vector>

class Contact
{
public:
    std::string name;
    std::string number;
    std::string firstname;
    std::string lastname;
    std::string phone;
    std::string mobile;
    std::string email;
    std::string address;
    std::string city;
    std::string state;
    std::string zip;
    std::string comment;
    std::string id;
    bool presence = false;
    bool starred = false;
    bool directory = false;
    std::string info;
    bool ringing = false;
    int image = 0;
    bool candidate = false;
};

struct ContactWithFields {
    Contact contact;
    std::vector<std::string> fields;
    bool processed = false;
};
