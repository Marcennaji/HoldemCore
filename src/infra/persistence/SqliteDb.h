#pragma once

#include "SqliteStatement.h"

#include <filesystem>
#include <string>

#define SQL_LOG_FILE "HoldemCore.db"

struct sqlite3;

namespace pkt::infra
{

class SqliteDb
{
  public:
    explicit SqliteDb(const std::filesystem::path& file);
    ~SqliteDb();

    void exec(const std::string& sql);
    std::unique_ptr<SqliteStatement> prepare(const std::string& sql);

  private:
    sqlite3* myDb{nullptr};
};

} // namespace pkt::infra
