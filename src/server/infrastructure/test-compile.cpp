#include "infrastructure/database/sqlite/schema.h"
#include "infrastructure/database/sqlite/database-factory.h"
#include "infrastructure/database/sqlite/sqlite-database.h"

#include <boost/asio/thread_pool.hpp>
#include <iostream>

int main() {
    auto xd1 = boost::asio::thread_pool(10);

    auto xd2 = infrastructure::db::sqlite::DatabaseFactory("asdasd.db", 1);
    auto xd3 = infrastructure::db::sqlite::SqliteDatabase(xd1, xd2);

    int a; std::cin >> a;
}