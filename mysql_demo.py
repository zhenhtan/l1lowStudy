import os

import mysql.connector
from mysql.connector import Error


def print_table(cursor, title, query):
    print(title)
    cursor.execute(query)
    rows = cursor.fetchall()
    if not rows:
        print("(empty)\n")
        return

    for row in rows:
        print(row)
    print()


def setup_schema(cursor):
    cursor.execute(
        """
        CREATE TABLE IF NOT EXISTS demo_users (
            id INT PRIMARY KEY,
            name VARCHAR(100) NOT NULL,
            email VARCHAR(255) NOT NULL UNIQUE
        )
        """
    )

    cursor.execute(
        """
        CREATE TABLE IF NOT EXISTS demo_orders (
            id INT PRIMARY KEY,
            user_id INT NOT NULL,
            created_at DATETIME NOT NULL,
            CONSTRAINT fk_demo_orders_user FOREIGN KEY (user_id) REFERENCES demo_users(id)
        )
        """
    )

    cursor.execute(
        """
        CREATE TABLE IF NOT EXISTS demo_order_items (
            id INT PRIMARY KEY,
            order_id INT NOT NULL,
            product_name VARCHAR(100) NOT NULL,
            quantity INT NOT NULL,
            unit_price DECIMAL(10, 2) NOT NULL,
            CONSTRAINT fk_demo_items_order FOREIGN KEY (order_id) REFERENCES demo_orders(id)
        )
        """
    )


def seed_data(cursor):
    users_sql = (
        "INSERT INTO demo_users (id, name, email) VALUES (%s, %s, %s) "
        "ON DUPLICATE KEY UPDATE name = VALUES(name), email = VALUES(email)"
    )
    users_data = [
        (1, "Alice", "alice@example.com"),
        (2, "Bob", "bob@example.com"),
    ]
    cursor.executemany(users_sql, users_data)
    print(f"users upsert rowcount: {cursor.rowcount}")

    orders_sql = (
        "INSERT INTO demo_orders (id, user_id, created_at) VALUES (%s, %s, %s) "
        "ON DUPLICATE KEY UPDATE user_id = VALUES(user_id), created_at = VALUES(created_at)"
    )
    orders_data = [
        (101, 1, "2026-04-10 09:00:00"),
        (102, 1, "2026-04-11 15:30:00"),
        (201, 2, "2026-04-12 10:10:00"),
    ]
    cursor.executemany(orders_sql, orders_data)
    print(f"orders upsert rowcount: {cursor.rowcount}")

    items_sql = (
        "INSERT INTO demo_order_items (id, order_id, product_name, quantity, unit_price) "
        "VALUES (%s, %s, %s, %s, %s) "
        "ON DUPLICATE KEY UPDATE "
        "order_id = VALUES(order_id), "
        "product_name = VALUES(product_name), "
        "quantity = VALUES(quantity), "
        "unit_price = VALUES(unit_price)"
    )
    items_data = [
        (1001, 101, "Keyboard", 1, 199.00),
        (1002, 101, "Mouse", 2, 59.00),
        (1003, 102, "Monitor", 1, 999.00),
        (1004, 201, "Headset", 1, 299.00),
        (1005, 201, "Mousepad", 1, 39.00),
    ]
    cursor.executemany(items_sql, items_data)
    print(f"order_items upsert rowcount: {cursor.rowcount}")


def main():
    conn = None
    cursor = None
    db_host = os.getenv("MYSQL_HOST", "127.0.0.1")
    db_port = int(os.getenv("MYSQL_PORT", "3306"))
    db_user = os.getenv("MYSQL_USER", "demo")
    db_password = os.getenv("MYSQL_PASSWORD", "demo123")
    db_name = os.getenv("MYSQL_DATABASE", "demo_db")
    try:
        # 1) 连接数据库
        conn = mysql.connector.connect(
            host=db_host,
            port=db_port,
            user=db_user,
            password=db_password,
            database=db_name,
        )
        cursor = conn.cursor()

        setup_schema(cursor)
        seed_data(cursor)
        conn.commit()

        print_table(cursor, "demo_users:", "SELECT id, name, email FROM demo_users ORDER BY id")
        print_table(
            cursor,
            "demo_orders:",
            "SELECT id, user_id, created_at FROM demo_orders ORDER BY id",
        )
        print_table(
            cursor,
            "demo_order_items:",
            "SELECT id, order_id, product_name, quantity, unit_price FROM demo_order_items ORDER BY id",
        )

        join_sql = """
            SELECT
                u.id AS user_id,
                u.name AS user_name,
                o.id AS order_id,
                o.created_at,
                SUM(oi.quantity * oi.unit_price) AS order_amount
            FROM demo_users u
            JOIN demo_orders o
                ON o.user_id = u.id
            JOIN demo_order_items oi
                ON oi.order_id = o.id
            WHERE o.created_at >= %s
              AND o.created_at < %s
            GROUP BY
                u.id, u.name, o.id, o.created_at
            ORDER BY
                order_amount DESC
        """
        cursor.execute(join_sql, ("2026-04-11", "2026-05-01"))
        join_rows = cursor.fetchall()
        print("join result (user + order + order_items):")
        if not join_rows:
            print("(empty)")
        else:
            for row in join_rows:
                print(row)

        print("Done.")
    except Error as e:
        print(f"MySQL Error: {e}")
        if conn:
            conn.rollback()
    finally:
        if cursor:
            cursor.close()
        if conn and conn.is_connected():
            conn.close()


if __name__ == "__main__":
    main()