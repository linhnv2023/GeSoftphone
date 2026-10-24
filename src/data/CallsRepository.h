#pragma once

#include "ICallsRepository.h"
#include <SQLiteCpp/SQLiteCpp.h>

class CallsRepository : public ICallsRepository
{
public:
    explicit CallsRepository(SQLite::Database& db);
    
    void BeginTransaction() override;
    void Commit() override;
    void Rollback() override;    

    int Add(const Call& call) override;
    void Update(const Call& call) override;
    void Delete(int id) override;
    void DeleteByIds(const std::vector<int>& ids) override;
    void DeleteAll() override;

    std::unique_ptr<Call> GetById(int id) override;
    std::unique_ptr<Call> GetByCallId(const std::string& callId) override;
    std::vector<std::unique_ptr<Call>> GetAll(int limit = 0) override;

private:
    SQLite::Database& m_db;
    std::unique_ptr<Call> MapQueryToCall(SQLite::Statement& query);
    void BindCallFields(SQLite::Statement& query, const Call& call, bool includeId);
};
