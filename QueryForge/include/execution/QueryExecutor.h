#ifndef QUERYEXECUTOR_H
#define QUERYEXECUTOR_H

#include <string>
#include "core/Database.h"

using namespace std;

class QueryExecutor {
public:
    explicit QueryExecutor(Database& database);

    void execute(const string& query);

private:
    Database& database;

    void executeCreate(const string& query);
    void executeInsert(const string& query);
    void executeSelect(const string& query);
    void executeUpdate(const string& query);
    void executeDelete(const string& query);
};

#endif