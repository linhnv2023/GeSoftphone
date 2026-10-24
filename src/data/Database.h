#pragma once

#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <string>

class Database
{
public:
    explicit Database(const std::string& filename);
    SQLite::Database& Get();
    void InitCalls();
    void InitContacts();
private:
    std::unique_ptr<SQLite::Database> m_db;
};
