#include "Database.h"

Database::Database(const std::string& filename)
{
    m_db = std::make_unique<SQLite::Database>(
        filename,
        SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE
    );
    Get().exec("PRAGMA busy_timeout=5000;");
    Get().exec("PRAGMA journal_mode=WAL;");
    Get().exec("PRAGMA synchronous=NORMAL;");
}

SQLite::Database& Database::Get()
{
    return *m_db;
}

void Database::InitCalls()
{
    Get().exec(
        "CREATE TABLE IF NOT EXISTS call_log ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT"
        ",callId TEXT NOT NULL"
        ",number TEXT NOT NULL"
        ",name TEXT NOT NULL"
        ",type INTEGER"
        ",time INTEGER"
        ",duration INTEGER"
        ",info TEXT NOT NULL"
        ");"
    );
    // Remove buggy unique constraint if it was previously created
    Get().exec("DROP INDEX IF EXISTS idx_call_log_callId_unique;");
    Get().exec("CREATE INDEX IF NOT EXISTS idx_call_log_callId ON call_log(callId);");
    Get().exec("CREATE INDEX IF NOT EXISTS idx_call_log_time ON call_log(time DESC);");
    Get().exec("CREATE INDEX IF NOT EXISTS idx_call_log_number ON call_log(number);");

    Get().exec("DELETE FROM call_log "
        "WHERE (SELECT COUNT(*) FROM call_log) > 1000 "
        "AND id <= ("
            "SELECT id FROM call_log "
            "ORDER BY id DESC "
            "LIMIT 1 OFFSET 1000"
        ");"
    );
}

void Database::InitContacts()
{
    Get().exec(
        "CREATE TABLE IF NOT EXISTS contacts ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT"
        ",number TEXT NOT NULL"
        ",name TEXT NOT NULL"
        ",info TEXT NOT NULL"
        ");"
    );
    Get().exec("CREATE INDEX IF NOT EXISTS idx_contacts_name ON contacts(name);");
    Get().exec("CREATE INDEX IF NOT EXISTS idx_contacts_number ON contacts(number);");
}
