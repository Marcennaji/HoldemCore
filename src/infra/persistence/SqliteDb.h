#pragma once

#include "SqliteStatement.h"

#include <filesystem>
#include <string>

#define SQL_LOG_FILE "HoldemCore.db"

struct sqlite3;

namespace pkt::infra
{

/**
 * @brief SQLite database wrapper for persistent data storage.
 * 
 * Provides a simplified interface for SQLite database operations including
 * connection management, SQL execution, and prepared statement creation
 * for poker engine data persistence needs.
 */
class SqliteDb
{
  public:
    explicit SqliteDb(const std::filesystem::path& file);
    ~SqliteDb();

    void exec(const std::string& sql);
    std::unique_ptr<SqliteStatement> prepare(const std::string& sql);

  private:
    sqlite3* m_db{nullptr};
};

} // namespace pkt::infra
