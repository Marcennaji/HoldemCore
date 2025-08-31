#include "SqliteDb.h"

#include <stdexcept>
#include <string>

namespace pkt::infra
{

SqliteDb::SqliteDb(const std::filesystem::path& file)
{
    int rc = sqlite3_open(file.string().c_str(), &myDb);
    if (rc != SQLITE_OK)
    {
        std::string msg = sqlite3_errmsg(myDb);
        sqlite3_close(myDb);
        myDb = nullptr;
        throw std::runtime_error("Failed to open SQLite database: " + msg);
    }
}

SqliteDb::~SqliteDb()
{
    if (myDb)
    {
        sqlite3_close(myDb);
        myDb = nullptr;
    }
}

void SqliteDb::exec(const std::string& sql)
{
    char* errMsg = nullptr;
    int rc = sqlite3_exec(myDb, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        std::string msg = errMsg ? errMsg : "Unknown SQLite error";
        sqlite3_free(errMsg);
        throw std::runtime_error("SQLite exec failed: " + msg);
    }
}

std::unique_ptr<SqliteStatement> SqliteDb::prepare(const std::string& sql)
{
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(myDb, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error("SQLite prepare failed: " + std::string(sqlite3_errmsg(myDb)));
    }
    return std::make_unique<SqliteStatement>(stmt);
}
} // namespace pkt::infra
