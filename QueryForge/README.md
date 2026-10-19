# QueryForge

QueryForge is a small C++ mini database engine. It supports tables with INTEGER, FLOAT, and TEXT columns.

## Build (GCC / MinGW)

Run these commands from the project folder:

```powershell
g++ -std=c++17 -Iinclude src/main.cpp src/core/Column.cpp src/core/Value.cpp src/core/Record.cpp src/core/Table.cpp src/Database.cpp src/execution/QueryExecutor.cpp src/storage/FileStorageEngine.cpp -o QueryForge.exe
.\QueryForge.exe
```

## Supported commands

```sql
CREATE TABLE students (id INTEGER, name TEXT, marks FLOAT);
INSERT INTO students VALUES (1, 'Payal', 85.5);
SELECT * FROM students;
SELECT * FROM students WHERE marks > 80;
UPDATE students SET marks = 95.0 WHERE id = 1;
DELETE FROM students WHERE id = 1;
EXIT
```

Tables and records are automatically saved to `data/queryforge.db` and loaded the next time the program starts.

Current limits: one-line queries, `SELECT *` only, and one condition for `WHERE`, `UPDATE`, and `DELETE`.

## OOP concepts

- Encapsulation: classes keep their data private and expose public methods.
- Abstraction: `StorageEngine` defines save/load operations without their implementation.
- Inheritance: `FileStorageEngine` inherits from `StorageEngine`.
- Polymorphism: `StorageEngine` virtual methods are implemented by `FileStorageEngine` and called through a base-class reference.
