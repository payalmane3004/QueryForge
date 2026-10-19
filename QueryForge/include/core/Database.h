
#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <unordered_map>
#include "core/Table.h"

using namespace std;

class Database {
public:
    Database();

    void createTable(const string& name, const vector<Column>& columns);

    Table& getTable(const string& name);
    const Table& getTable(const string& name) const;

    bool hasTable(const string& name) const;

    vector<string> getTableNames() const;

    int getTableCount() const;

    void saveToFile(const string& filename) const;
    void loadFromFile(const string& filename);

private:
    unordered_map<string, Table> tables;
};

#endif

