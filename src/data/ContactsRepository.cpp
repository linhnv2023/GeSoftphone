#include "ContactsRepository.h"

ContactsRepository::ContactsRepository(SQLite::Database& db)
    : m_db(db)
{
}

void ContactsRepository::BeginTransaction()
{
    try {
        m_db.exec("BEGIN TRANSACTION;");
    } catch (...) {}
}

void ContactsRepository::Commit()
{
    try {
        m_db.exec("COMMIT;");
    } catch (...) {}
}

void ContactsRepository::Rollback()
{
    try {
        m_db.exec("ROLLBACK;");
    } catch (...) {}
}

int ContactsRepository::Add(const Contact& contact)
{
    try {
        SQLite::Statement query(m_db,
            "INSERT INTO contacts (number, name, info) VALUES (?, ?, ?);");

        BindContactFields(query, contact, false);
        query.exec();

        return static_cast<int>(m_db.getLastInsertRowid());
    } catch (...) {
        return -1;
    }
}

void ContactsRepository::Update(const Contact& contact)
{
    try {
        SQLite::Statement query(m_db,
            "UPDATE contacts SET number=?, name=?, info=? WHERE number=?;");

        BindContactFields(query, contact, true);
        query.exec();
    } catch (...) {}
}

void ContactsRepository::Delete(int id)
{
    try {
        SQLite::Statement query(m_db,
            "DELETE FROM contacts WHERE id=?;");
        query.bind(1, id);
        query.exec();
    } catch (...) {}
}

void ContactsRepository::DeleteAll()
{
    try {
        m_db.exec("DELETE FROM contacts;");
    } catch (...) {}
}

std::unique_ptr<Contact> ContactsRepository::GetById(int id)
{
    try {
        SQLite::Statement query(m_db,
            "SELECT * FROM contacts WHERE id=?;");
        query.bind(1, id);

        if (query.executeStep())
            return MapQueryToContact(query);
    } catch (...) {}

    return nullptr;
}

std::unique_ptr<Contact> ContactsRepository::GetByContactId(const std::string& contactId)
{
    try {
        SQLite::Statement query(m_db,
            "SELECT * FROM contacts WHERE number=?;");
        query.bind(1, contactId);

        if (query.executeStep())
            return MapQueryToContact(query);
    } catch (...) {}

    return nullptr;
}

std::vector<std::unique_ptr<Contact>> ContactsRepository::GetAll()
{
    std::vector<std::unique_ptr<Contact>> result;
    try {
        SQLite::Statement query(m_db, "SELECT * FROM contacts ORDER BY name ASC;");

        while (query.executeStep())
        {
            result.push_back(MapQueryToContact(query));
        }
    } catch (...) {}

    return result;
}

std::unique_ptr<Contact> ContactsRepository::MapQueryToContact(SQLite::Statement& query)
{
    auto contact = std::make_unique<Contact>();
    contact->number = query.getColumn("number").getString();
    contact->name = query.getColumn("name").getString();
    contact->info = query.getColumn("info").getString();
    return contact;
}

void ContactsRepository::BindContactFields(SQLite::Statement& query, const Contact& contact, bool includeId)
{
    int index = 1;
    query.bind(index++, contact.number);
    query.bind(index++, contact.name);
    query.bind(index++, contact.info);
    if (includeId) {
        query.bind(index++, contact.number);
    }
}
