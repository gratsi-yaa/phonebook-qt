#include "PostgresStorage.h"
#include <QDate>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <map>
#include <stdexcept>
#include "QtUtil.h"

namespace
{

    void check(bool ok, const QSqlQuery &query, const char *what)
    {
        if (!ok)
            throw std::runtime_error(std::string(what) + ": " + toStd(query.lastError().text()));
    }

    void bindContact(QSqlQuery &q, const Contact &c)
    {
        q.addBindValue(fromStd(c.lastName));
        q.addBindValue(fromStd(c.firstName));
        q.addBindValue(fromStd(c.middleName));
        q.addBindValue(fromStd(c.address));
        q.addBindValue(fromStd(c.email));
        if (c.birthDate)
            q.addBindValue(QDate(c.birthDate->year, c.birthDate->month, c.birthDate->day));
        else
            q.addBindValue(QVariant(QMetaType::fromType<QDate>())); // SQL NULL
    }

    // Commits on success, rolls back if the body throws.
    template <typename F>
    void inTransaction(QSqlDatabase &db, F body)
    {
        if (!db.transaction())
            throw std::runtime_error("Cannot start transaction: " + toStd(db.lastError().text()));
        try
        {
            body();
        }
        catch (...)
        {
            db.rollback();
            throw;
        }
        if (!db.commit())
            throw std::runtime_error("Cannot commit: " + toStd(db.lastError().text()));
    }

} // namespace

PostgresStorage::PostgresStorage(const Settings &s)
{
    if (!QSqlDatabase::isDriverAvailable("QPSQL"))
        throw std::runtime_error(
            "The Qt PostgreSQL driver (QPSQL) is not available. Available drivers: " +
            toStd(QSqlDatabase::drivers().join(", ")) +
            ". Make sure the 'bin' folder of PostgreSQL (it contains libpq.dll) is in PATH.");

    m_connectionName = QString("phonebook_%1").arg(reinterpret_cast<quintptr>(this));
    m_db = QSqlDatabase::addDatabase("QPSQL", m_connectionName);
    m_db.setHostName(s.host);
    m_db.setPort(s.port);
    m_db.setDatabaseName(s.database);
    m_db.setUserName(s.user);
    m_db.setPassword(s.password);

    if (!m_db.open())
    {
        std::string message = toStd(m_db.lastError().text());
        m_db = QSqlDatabase();
        QSqlDatabase::removeDatabase(m_connectionName);
        throw std::runtime_error("Cannot connect to PostgreSQL: " + message);
    }
    ensureSchema();
}

PostgresStorage::~PostgresStorage()
{
    m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connectionName);
}

void PostgresStorage::ensureSchema()
{
    const QStringList statements = {
        "CREATE TABLE IF NOT EXISTS contacts ("
        " id SERIAL PRIMARY KEY,"
        " last_name TEXT NOT NULL,"
        " first_name TEXT NOT NULL,"
        " middle_name TEXT NOT NULL DEFAULT '',"
        " address TEXT NOT NULL DEFAULT '',"
        " email TEXT NOT NULL,"
        " birth_date DATE)",
        "CREATE TABLE IF NOT EXISTS phones ("
        " id SERIAL PRIMARY KEY,"
        " contact_id INTEGER NOT NULL REFERENCES contacts(id) ON DELETE CASCADE,"
        " type TEXT NOT NULL CHECK (type IN ('work', 'home', 'service')),"
        " number TEXT NOT NULL)",
        "CREATE INDEX IF NOT EXISTS idx_phones_contact ON phones(contact_id)"};
    for (const QString &sql : statements)
    {
        QSqlQuery q(m_db);
        check(q.exec(sql), q, "Cannot create the tables");
    }
}

std::vector<Contact> PostgresStorage::loadAll()
{
    std::vector<Contact> result;
    std::map<int, std::size_t> indexById;

    QSqlQuery q(m_db);
    check(q.exec("SELECT id, last_name, first_name, middle_name, address, email, birth_date "
                 "FROM contacts ORDER BY id"),
          q, "Cannot load contacts");
    while (q.next())
    {
        Contact c;
        c.id = q.value(0).toInt();
        c.lastName = toStd(q.value(1).toString());
        c.firstName = toStd(q.value(2).toString());
        c.middleName = toStd(q.value(3).toString());
        c.address = toStd(q.value(4).toString());
        c.email = toStd(q.value(5).toString());
        QDate d = q.value(6).toDate();
        if (d.isValid())
            c.birthDate = Date{d.year(), d.month(), d.day()};
        indexById[c.id] = result.size();
        result.push_back(c);
    }

    QSqlQuery p(m_db);
    check(p.exec("SELECT contact_id, type, number FROM phones ORDER BY id"), p,
          "Cannot load phone numbers");
    while (p.next())
    {
        auto it = indexById.find(p.value(0).toInt());
        if (it == indexById.end())
            continue;
        PhoneNumber phone;
        if (!phoneTypeFromString(toStd(p.value(1).toString()), phone.type))
            throw std::runtime_error("Unknown phone type in the database");
        phone.number = toStd(p.value(2).toString());
        result[it->second].phones.push_back(phone);
    }
    return result;
}

void PostgresStorage::insertPhones(int contactId, const std::vector<PhoneNumber> &phones)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO phones (contact_id, type, number) VALUES (?, ?, ?)");
    for (const PhoneNumber &ph : phones)
    {
        q.addBindValue(contactId);
        q.addBindValue(QString::fromLatin1(toString(ph.type)));
        q.addBindValue(fromStd(ph.number));
        check(q.exec(), q, "Cannot insert a phone number");
    }
}

int PostgresStorage::add(const Contact &c)
{
    int id = 0;
    inTransaction(m_db, [&]
                  {
        QSqlQuery q(m_db);
        q.prepare("INSERT INTO contacts (last_name, first_name, middle_name, address, "
                  "email, birth_date) VALUES (?, ?, ?, ?, ?, ?) RETURNING id");
        bindContact(q, c);
        check(q.exec() && q.next(), q, "Cannot insert the contact");
        id = q.value(0).toInt();
        insertPhones(id, c.phones); });
    return id;
}

void PostgresStorage::update(const Contact &c)
{
    inTransaction(m_db, [&]
                  {
        QSqlQuery q(m_db);
        q.prepare("UPDATE contacts SET last_name = ?, first_name = ?, middle_name = ?, "
                  "address = ?, email = ?, birth_date = ? WHERE id = ?");
        bindContact(q, c);
        q.addBindValue(c.id);
        check(q.exec(), q, "Cannot update the contact");
        if (q.numRowsAffected() != 1) throw std::runtime_error("Contact not found");

        QSqlQuery del(m_db);
        del.prepare("DELETE FROM phones WHERE contact_id = ?");
        del.addBindValue(c.id);
        check(del.exec(), del, "Cannot update the phone numbers");
        insertPhones(c.id, c.phones); });
}

void PostgresStorage::remove(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM contacts WHERE id = ?"); // phones are deleted by ON DELETE CASCADE
    q.addBindValue(id);
    check(q.exec(), q, "Cannot delete the contact");
    if (q.numRowsAffected() != 1)
        throw std::runtime_error("Contact not found");
}