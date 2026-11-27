-- Phone book database schema (PostgreSQL).
-- PostgresStorage runs the same statements at startup if the tables are missing.

CREATE TABLE IF NOT EXISTS contacts (
    id          SERIAL PRIMARY KEY,
    last_name   TEXT NOT NULL,
    first_name  TEXT NOT NULL,
    middle_name TEXT NOT NULL DEFAULT '',
    address     TEXT NOT NULL DEFAULT '',
    email       TEXT NOT NULL,
    birth_date  DATE
);

CREATE TABLE IF NOT EXISTS phones (
    id         SERIAL PRIMARY KEY,
    contact_id INTEGER NOT NULL REFERENCES contacts(id) ON DELETE CASCADE,
    type       TEXT NOT NULL CHECK (type IN ('work', 'home', 'service')),
    number     TEXT NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_phones_contact ON phones(contact_id);