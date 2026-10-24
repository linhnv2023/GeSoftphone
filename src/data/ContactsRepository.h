#pragma once

#include "IContactsRepository.h"
#include <SQLiteCpp/SQLiteCpp.h>

class ContactsRepository : public IContactsRepository
{
public:
    explicit ContactsRepository(SQLite::Database& db);

    void BeginTransaction() override;
    void Commit() override;
    void Rollback() override;

    int Add(const Contact& contact) override;
    void Update(const Contact& contact) override;
    void Delete(int id) override;
    void DeleteAll() override;

    std::unique_ptr<Contact> GetById(int id) override;
    std::unique_ptr<Contact> GetByContactId(const std::string& contactId) override;
    std::vector<std::unique_ptr<Contact>> GetAll() override;

private:
    SQLite::Database& m_db;
    std::unique_ptr<Contact> MapQueryToContact(SQLite::Statement& query);
    void BindContactFields(SQLite::Statement& query, const Contact& contact, bool includeId);
};
