#pragma once

#include <vector>
#include <memory>
#include <string>
#include "../domain/Call.h"

class ICallsRepository
{
public:
    virtual ~ICallsRepository() = default;

    virtual void BeginTransaction() = 0;
    virtual void Commit() = 0;
    virtual void Rollback() = 0;

    virtual int Add(const Call& call) = 0;
    virtual void Update(const Call& call) = 0;
    virtual void Delete(int id) = 0;
    virtual void DeleteByIds(const std::vector<int>& ids) = 0;
    virtual void DeleteAll() = 0;

    virtual std::unique_ptr<Call> GetById(int id) = 0;
    virtual std::unique_ptr<Call> GetByCallId(const std::string& callId) = 0;
    virtual std::vector<std::unique_ptr<Call>> GetAll(int limit = 0) = 0;
};
