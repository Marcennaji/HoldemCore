#pragma once

#include <stdexcept>
#include <string>
#include <third_party/sqlite3/sqlite3.h>

struct sqlite3_stmt;

class SqliteStatement
{
  public:
    explicit SqliteStatement(sqlite3_stmt* stmt) : myStmt(stmt)
    {
        if (!myStmt)
        {
            throw std::runtime_error("Null sqlite3_stmt in SqliteStatement");
        }
    }

    ~SqliteStatement()
    {
        if (myStmt)
        {
            sqlite3_finalize(myStmt);
            myStmt = nullptr;
        }
    }

    SqliteStatement(const SqliteStatement&) = delete;
    SqliteStatement& operator=(const SqliteStatement&) = delete;

    SqliteStatement(SqliteStatement&& other) noexcept : myStmt(other.myStmt) { other.myStmt = nullptr; }

    SqliteStatement& operator=(SqliteStatement&& other) noexcept
    {
        if (this != &other)
        {
            if (myStmt)
            {
                sqlite3_finalize(myStmt);
            }
            myStmt = other.myStmt;
            other.myStmt = nullptr;
        }
        return *this;
    }

    // parameters binding  (index 1-based)
    void bindInt(int index, int value) { check(sqlite3_bind_int(myStmt, index, value)); }

    void bindInt64(int index, sqlite3_int64 value) { check(sqlite3_bind_int64(myStmt, index, value)); }

    void bindDouble(int index, double value) { check(sqlite3_bind_double(myStmt, index, value)); }

    void bindText(int index, const std::string& value)
    {
        check(sqlite3_bind_text(myStmt, index, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT));
    }

    void bindNull(int index) { check(sqlite3_bind_null(myStmt, index)); }

    // Execution: step returns true if a row is available
    bool step()
    {
        int rc = sqlite3_step(myStmt);
        if (rc == SQLITE_ROW)
        {
            return true;
        }
        if (rc == SQLITE_DONE)
        {
            return false;
        }
        throw std::runtime_error("SQLite step failed: " + std::string(sqlite3_errmsg(sqlite3_db_handle(myStmt))));
    }

    void reset() { check(sqlite3_reset(myStmt)); }

    // column reading (index 0-based)
    int getInt(int col) const { return sqlite3_column_int(myStmt, col); }

    sqlite3_int64 getInt64(int col) const { return sqlite3_column_int64(myStmt, col); }

    double getDouble(int col) const { return sqlite3_column_double(myStmt, col); }

    std::string getText(int col) const
    {
        const unsigned char* text = sqlite3_column_text(myStmt, col);
        if (!text)
            return {};
        return reinterpret_cast<const char*>(text);
    }

    bool isNull(int col) const { return sqlite3_column_type(myStmt, col) == SQLITE_NULL; }

  private:
    sqlite3_stmt* myStmt{nullptr};

    void check(int rc)
    {
        if (rc != SQLITE_OK)
        {
            throw std::runtime_error("SQLite bind/reset error: " + std::to_string(rc));
        }
    }
};
