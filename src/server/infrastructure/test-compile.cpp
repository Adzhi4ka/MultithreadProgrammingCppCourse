#include "infrastructure/database/sqlite/schema.h"
#include "infrastructure/database/sqlite/database-factory.h"
#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/database/repositories/user-repository.h"
#include "infrastructure/database/repositories/group-repository.h"
#include "infrastructure/database/repositories/file-repository.h"
#include "infrastructure/database/repositories/file-acl-repository.h"
#include "infrastructure/database/repositories/file-lock-repository.h"

#include <boost/asio/thread_pool.hpp>

int main() {
    auto xd1 = boost::asio::thread_pool(10);

    auto xd2 = infrastructure::db::sqlite::DatabaseFactory("asdasd", 123);
    auto xd3 = infrastructure::db::sqlite::SqliteDatabase(xd1, xd2);
    auto xd4 = infrastructure::db::repositories::UserRepository(xd3);
}