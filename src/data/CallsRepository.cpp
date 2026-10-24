#include "CallsRepository.h"

CallsRepository::CallsRepository(SQLite::Database& db)
    : m_db(db)
{
}

void CallsRepository::BeginTransaction()
{
    try {
        m_db.exec("BEGIN TRANSACTION;");
    } catch (...) {}
}

void CallsRepository::Commit()
{
    try {
        m_db.exec("COMMIT;");
    } catch (...) {}
}

void CallsRepository::Rollback()
{
    try {
        m_db.exec("ROLLBACK;");
    } catch (...) {}
}

int CallsRepository::Add(const Call& call)
{
    try {
        SQLite::Statement query(m_db,
            "INSERT INTO call_log "
            "(callId, number, name, type, time, duration, info) "
            "VALUES (?, ?, ?, ?, ?, ?, ?);");

        BindCallFields(query, call, false);
        query.exec();

        return static_cast<int>(m_db.getLastInsertRowid());
    } catch (...) {
        return -1;
    }
}

void CallsRepository::Update(const Call& call)
{
    try {
        SQLite::Statement query(m_db,
            "UPDATE call_log SET "
            "callId=?, number=?, name=?, type=?, time=?, duration=?, info=? "
            "WHERE id=?;");

        BindCallFields(query, call, true);
        query.exec();
    } catch (...) {}
}

void CallsRepository::Delete(int id)
{
    try {
        SQLite::Statement query(m_db,
            "DELETE FROM call_log WHERE id=?;");
        query.bind(1, id);
        query.exec();
    } catch (...) {}
}

void CallsRepository::DeleteByIds(const std::vector<int>& ids)
{
    if (ids.empty()) return;

    try {
        std::string str;
        for (size_t i = 0; i < ids.size(); ++i)
        {
            if (i > 0)
                str += ",";
            str += std::to_string(ids[i]);
        }
        SQLite::Statement query(m_db,
            "DELETE FROM call_log WHERE id IN (" + str + ");");
        query.exec();
    } catch (...) {}
}

void CallsRepository::DeleteAll()
{
    try {
        m_db.exec("DELETE FROM call_log;");
    } catch (...) {}
}

std::unique_ptr<Call> CallsRepository::GetById(int id)
{
    try {
        SQLite::Statement query(m_db,
            "SELECT * FROM call_log WHERE id=?;");
        query.bind(1, id);

        if (query.executeStep())
            return MapQueryToCall(query);
    } catch (...) {}

    return nullptr;
}

std::unique_ptr<Call> CallsRepository::GetByCallId(const std::string& callId)
{
    try {
        SQLite::Statement query(m_db,
            "SELECT * FROM call_log WHERE callId=?;");
        query.bind(1, callId);

        if (query.executeStep())
            return MapQueryToCall(query);
    } catch (...) {}

    return nullptr;
}

std::vector<std::unique_ptr<Call>> CallsRepository::GetAll(int limit)
{
    std::vector<std::unique_ptr<Call>> result;
    try {
        std::string str;
        if (limit > 0) {
            str = " LIMIT " + std::to_string(limit);
        }
        SQLite::Statement query(m_db, "SELECT * FROM call_log ORDER BY time DESC" + str + ";");
        while (query.executeStep())
        {
            result.push_back(MapQueryToCall(query));
        }
    } catch (...) {}
    return result;
}

std::unique_ptr<Call> CallsRepository::MapQueryToCall(SQLite::Statement& query)
{
    auto call = std::make_unique<Call>();
    call->id = query.getColumn("id").getInt();
    call->callId = query.getColumn("callId").getString();
    call->number = query.getColumn("number").getString();
    call->name = query.getColumn("name").getString();
    call->type = query.getColumn("type").getInt();
    call->time = query.getColumn("time").getInt64();
    call->duration = query.getColumn("duration").getInt();
    call->info = query.getColumn("info").getString();
    return call;
}

void CallsRepository::BindCallFields(SQLite::Statement& query, const Call& call, bool includeId)
{
    int index = 1;

    query.bind(index++, call.callId);
    query.bind(index++, call.number);
    query.bind(index++, call.name);
    query.bind(index++, call.type);
    query.bind(index++, call.time);
    query.bind(index++, call.duration);
    query.bind(index++, call.info);

    if (includeId) {
        query.bind(index++, call.id);
    }
}
