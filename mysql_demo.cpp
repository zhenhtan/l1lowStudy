#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#include <mysql/mysql.h>

namespace {

std::string getenvOrDefault(const char* key, const char* defaultValue) {
    const char* val = std::getenv(key);
    return (val && *val) ? std::string(val) : std::string(defaultValue);
}

void execOrThrow(MYSQL* conn, const std::string& sql) {
    if (mysql_query(conn, sql.c_str()) != 0) {
        throw std::runtime_error(mysql_error(conn));
    }
}

void printTable(MYSQL* conn, const std::string& title, const std::string& query) {
    std::cout << title << "\n";
    execOrThrow(conn, query);

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        if (mysql_field_count(conn) == 0) {
            std::cout << "(empty)\n\n";
            return;
        }
        throw std::runtime_error(mysql_error(conn));
    }

    MYSQL_ROW row;
    bool hasRows = false;
    while ((row = mysql_fetch_row(result)) != nullptr) {
        hasRows = true;
        unsigned int numFields = mysql_num_fields(result);
        std::cout << "(";
        for (unsigned int i = 0; i < numFields; ++i) {
            if (i > 0) {
                std::cout << ", ";
            }
            std::cout << (row[i] ? row[i] : "NULL");
        }
        std::cout << ")\n";
    }

    if (!hasRows) {
        std::cout << "(empty)\n";
    }
    std::cout << "\n";

    mysql_free_result(result);
}

void setupSchema(MYSQL* conn) {
    execOrThrow(
        conn,
        "CREATE TABLE IF NOT EXISTS demo_users ("
        "id INT PRIMARY KEY,"
        "name VARCHAR(100) NOT NULL,"
        "email VARCHAR(255) NOT NULL UNIQUE"
        ")");

    execOrThrow(
        conn,
        "CREATE TABLE IF NOT EXISTS demo_orders ("
        "id INT PRIMARY KEY,"
        "user_id INT NOT NULL,"
        "created_at DATETIME NOT NULL,"
        "CONSTRAINT fk_demo_orders_user FOREIGN KEY (user_id) REFERENCES demo_users(id)"
        ")");

    execOrThrow(
        conn,
        "CREATE TABLE IF NOT EXISTS demo_order_items ("
        "id INT PRIMARY KEY,"
        "order_id INT NOT NULL,"
        "product_name VARCHAR(100) NOT NULL,"
        "quantity INT NOT NULL,"
        "unit_price DECIMAL(10,2) NOT NULL,"
        "CONSTRAINT fk_demo_items_order FOREIGN KEY (order_id) REFERENCES demo_orders(id)"
        ")");
}

void seedData(MYSQL* conn) {
    execOrThrow(
        conn,
        "INSERT INTO demo_users (id, name, email) VALUES "
        "(1, 'Alice', 'alice@example.com'),"
        "(2, 'Bob', 'bob@example.com') "
        "ON DUPLICATE KEY UPDATE name=VALUES(name), email=VALUES(email)");
    std::cout << "users upsert rowcount: " << mysql_affected_rows(conn) << "\n";

    execOrThrow(
        conn,
        "INSERT INTO demo_orders (id, user_id, created_at) VALUES "
        "(101, 1, '2026-04-10 09:00:00'),"
        "(102, 1, '2026-04-11 15:30:00'),"
        "(201, 2, '2026-04-12 10:10:00') "
        "ON DUPLICATE KEY UPDATE user_id=VALUES(user_id), created_at=VALUES(created_at)");
    std::cout << "orders upsert rowcount: " << mysql_affected_rows(conn) << "\n";

    execOrThrow(
        conn,
        "INSERT INTO demo_order_items (id, order_id, product_name, quantity, unit_price) VALUES "
        "(1001, 101, 'Keyboard', 1, 199.00),"
        "(1002, 101, 'Mouse', 2, 59.00),"
        "(1003, 102, 'Monitor', 1, 999.00),"
        "(1004, 201, 'Headset', 1, 299.00),"
        "(1005, 201, 'Mousepad', 1, 39.00) "
        "ON DUPLICATE KEY UPDATE "
        "order_id=VALUES(order_id),"
        "product_name=VALUES(product_name),"
        "quantity=VALUES(quantity),"
        "unit_price=VALUES(unit_price)");
    std::cout << "order_items upsert rowcount: " << mysql_affected_rows(conn) << "\n";
}

void printJoinResult(MYSQL* conn, const std::string& startDate, const std::string& endDate) {
    const char* joinSql =
        "SELECT "
        "u.id AS user_id, "
        "u.name AS user_name, "
        "o.id AS order_id, "
        "o.created_at, "
        "SUM(oi.quantity * oi.unit_price) AS order_amount "
        "FROM demo_users u "
        "JOIN demo_orders o ON o.user_id = u.id "
        "JOIN demo_order_items oi ON oi.order_id = o.id "
        "WHERE o.created_at >= ? "
        "AND o.created_at < ? "
        "GROUP BY u.id, u.name, o.id, o.created_at "
        "ORDER BY order_amount DESC";

    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        throw std::runtime_error("mysql_stmt_init failed");
    }

    try {
        if (mysql_stmt_prepare(stmt, joinSql, std::strlen(joinSql)) != 0) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }

        MYSQL_BIND paramBinds[2] = {};
        unsigned long startLen = startDate.size();
        unsigned long endLen = endDate.size();

        paramBinds[0].buffer_type = MYSQL_TYPE_STRING;
        paramBinds[0].buffer = const_cast<char*>(startDate.c_str());
        paramBinds[0].buffer_length = startLen;
        paramBinds[0].length = &startLen;

        paramBinds[1].buffer_type = MYSQL_TYPE_STRING;
        paramBinds[1].buffer = const_cast<char*>(endDate.c_str());
        paramBinds[1].buffer_length = endLen;
        paramBinds[1].length = &endLen;

        if (mysql_stmt_bind_param(stmt, paramBinds) != 0) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }

        if (mysql_stmt_execute(stmt) != 0) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }

        if (mysql_stmt_store_result(stmt) != 0) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }

        int userId = 0;
        int orderId = 0;
        char userName[101] = {};
        char createdAt[20] = {};
        char orderAmount[32] = {};
        unsigned long userNameLen = 0;
        unsigned long createdAtLen = 0;
        unsigned long orderAmountLen = 0;
        bool isNull[5] = {};

        MYSQL_BIND resultBinds[5] = {};

        resultBinds[0].buffer_type = MYSQL_TYPE_LONG;
        resultBinds[0].buffer = &userId;
        resultBinds[0].is_null = &isNull[0];

        resultBinds[1].buffer_type = MYSQL_TYPE_STRING;
        resultBinds[1].buffer = userName;
        resultBinds[1].buffer_length = sizeof(userName);
        resultBinds[1].length = &userNameLen;
        resultBinds[1].is_null = &isNull[1];

        resultBinds[2].buffer_type = MYSQL_TYPE_LONG;
        resultBinds[2].buffer = &orderId;
        resultBinds[2].is_null = &isNull[2];

        resultBinds[3].buffer_type = MYSQL_TYPE_STRING;
        resultBinds[3].buffer = createdAt;
        resultBinds[3].buffer_length = sizeof(createdAt);
        resultBinds[3].length = &createdAtLen;
        resultBinds[3].is_null = &isNull[3];

        resultBinds[4].buffer_type = MYSQL_TYPE_STRING;
        resultBinds[4].buffer = orderAmount;
        resultBinds[4].buffer_length = sizeof(orderAmount);
        resultBinds[4].length = &orderAmountLen;
        resultBinds[4].is_null = &isNull[4];

        if (mysql_stmt_bind_result(stmt, resultBinds) != 0) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }

        std::cout << "join result (user + order + order_items):\n";
        bool hasRows = false;
        while (true) {
            const int fetchStatus = mysql_stmt_fetch(stmt);
            if (fetchStatus == MYSQL_NO_DATA) {
                break;
            }
            if (fetchStatus != 0 && fetchStatus != MYSQL_DATA_TRUNCATED) {
                throw std::runtime_error(mysql_stmt_error(stmt));
            }

            hasRows = true;
            std::cout << "(" << (isNull[0] ? "NULL" : std::to_string(userId)) << ", "
                      << (isNull[1] ? "NULL" : std::string(userName, userNameLen)) << ", "
                      << (isNull[2] ? "NULL" : std::to_string(orderId)) << ", "
                      << (isNull[3] ? "NULL" : std::string(createdAt, createdAtLen)) << ", "
                      << (isNull[4] ? "NULL" : std::string(orderAmount, orderAmountLen)) << ")\n";
        }

        if (!hasRows) {
            std::cout << "(empty)\n";
        }

        mysql_stmt_free_result(stmt);
        mysql_stmt_close(stmt);
    } catch (...) {
        mysql_stmt_close(stmt);
        throw;
    }
}

}  // namespace

int main() {
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "MySQL init failed\n";
        return 1;
    }

    const std::string host = getenvOrDefault("MYSQL_HOST", "127.0.0.1");
    const std::string portStr = getenvOrDefault("MYSQL_PORT", "3306");
    const std::string user = getenvOrDefault("MYSQL_USER", "demo");
    const std::string password = getenvOrDefault("MYSQL_PASSWORD", "demo123");
    const std::string database = getenvOrDefault("MYSQL_DATABASE", "demo_db");

    unsigned int port = 3306;
    try {
        port = static_cast<unsigned int>(std::stoul(portStr));
    } catch (...) {
        std::cerr << "Invalid MYSQL_PORT: " << portStr << "\n";
        mysql_close(conn);
        return 1;
    }

    if (!mysql_real_connect(
            conn,
            host.c_str(),
            user.c_str(),
            password.c_str(),
            database.c_str(),
            port,
            nullptr,
            0)) {
        std::cerr << "MySQL Error: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return 1;
    }

    try {
        mysql_autocommit(conn, 0);

        setupSchema(conn);
        seedData(conn);

        if (mysql_commit(conn) != 0) {
            throw std::runtime_error(mysql_error(conn));
        }

        printTable(conn, "demo_users:", "SELECT id, name, email FROM demo_users ORDER BY id");
        printTable(conn, "demo_orders:", "SELECT id, user_id, created_at FROM demo_orders ORDER BY id");
        printTable(
            conn,
            "demo_order_items:",
            "SELECT id, order_id, product_name, quantity, unit_price FROM demo_order_items ORDER BY id");

        printJoinResult(conn, "2026-04-11", "2026-05-01");

        std::cout << "Done.\n";
    } catch (const std::exception& ex) {
        std::cerr << "MySQL Error: " << ex.what() << "\n";
        mysql_rollback(conn);
        mysql_close(conn);
        return 1;
    }

    mysql_close(conn);
    return 0;
}
