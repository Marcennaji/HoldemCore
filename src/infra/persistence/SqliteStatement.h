// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

#include <stdexcept>
#include <string>
#include <third_party/sqlite3/sqlite3.h>

struct sqlite3_stmt;

/**
 * @brief RAII wrapper for SQLite prepared statements.
 * 
 * Provides a type-safe interface for SQLite prepared statements with
 * automatic resource management, parameter binding, and result retrieval
 * for database operations in the poker engine.
 */
class SqliteStatement
{
  public:
    explicit SqliteStatement(sqlite3_stmt* stmt) : m_stmt(stmt)
    {
        if (!m_stmt)
        {
            throw std::runtime_error("Null sqlite3_stmt in SqliteStatement");
        }
    }

    ~SqliteStatement()
    {
        if (m_stmt)
        {
            sqlite3_finalize(m_stmt);
            m_stmt = nullptr;
        }
    }

    SqliteStatement(const SqliteStatement&) = delete;
    SqliteStatement& operator=(const SqliteStatement&) = delete;

    SqliteStatement(SqliteStatement&& other) noexcept : m_stmt(other.m_stmt) { other.m_stmt = nullptr; }

    SqliteStatement& operator=(SqliteStatement&& other) noexcept
    {
        if (this != &other)
        {
            if (m_stmt)
            {
                sqlite3_finalize(m_stmt);
            }
            m_stmt = other.m_stmt;
            other.m_stmt = nullptr;
        }
        return *this;
    }

    // parameters binding  (index 1-based)
    void bindInt(int index, int value) { check(sqlite3_bind_int(m_stmt, index, value)); }

    void bindInt64(int index, sqlite3_int64 value) { check(sqlite3_bind_int64(m_stmt, index, value)); }

    void bindDouble(int index, double value) { check(sqlite3_bind_double(m_stmt, index, value)); }

    void bindText(int index, const std::string& value)
    {
        check(sqlite3_bind_text(m_stmt, index, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT));
    }

    void bindNull(int index) { check(sqlite3_bind_null(m_stmt, index)); }

    // Execution: step returns true if a row is available
    bool step()
    {
        int rc = sqlite3_step(m_stmt);
        if (rc == SQLITE_ROW)
        {
            return true;
        }
        if (rc == SQLITE_DONE)
        {
            return false;
        }
        throw std::runtime_error("SQLite step failed: " + std::string(sqlite3_errmsg(sqlite3_db_handle(m_stmt))));
    }

    void reset() { check(sqlite3_reset(m_stmt)); }

    // column reading (index 0-based)
    int getInt(int col) const { return sqlite3_column_int(m_stmt, col); }

    sqlite3_int64 getInt64(int col) const { return sqlite3_column_int64(m_stmt, col); }

    double getDouble(int col) const { return sqlite3_column_double(m_stmt, col); }

    std::string getText(int col) const
    {
        const unsigned char* text = sqlite3_column_text(m_stmt, col);
        if (!text)
            return {};
        return reinterpret_cast<const char*>(text);
    }

    bool isNull(int col) const { return sqlite3_column_type(m_stmt, col) == SQLITE_NULL; }

  private:
    sqlite3_stmt* m_stmt{nullptr};

    void check(int rc)
    {
        if (rc != SQLITE_OK)
        {
            throw std::runtime_error("SQLite bind/reset error: " + std::to_string(rc));
        }
    }
};
