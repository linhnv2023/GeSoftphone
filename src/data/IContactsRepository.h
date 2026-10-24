#pragma once

#include <vector>
#include <memory>
#include <string>
#include "../domain/Contact.h"

class IContactsRepository
{
public:
    virtual ~IContactsRepository() = default;

    virtual void BeginTransaction() = 0;
    virtual void Commit() = 0;
    virtual void Rollback() = 0;

    virtual int Add(const Contact& contact) = 0;
    virtual void Update(const Contact& contact) = 0;
    virtual void Delete(int id) = 0;
    virtual void DeleteAll() = 0;

    virtual std::unique_ptr<Contact> GetById(int id) = 0;
    virtual std::unique_ptr<Contact> GetByContactId(const std::string& contactId) = 0;
    virtual std::vector<std::unique_ptr<Contact>> GetAll() = 0;
};
