#ifndef STORAGEENGINE_H
#define STORAGEENGINE_H

#include <string>
using namespace std;

class Database;

class StorageEngine {
public:
    virtual ~StorageEngine() {}
    virtual void save(const Database& database, const string& filename) = 0;
    virtual void load(Database& database, const string& filename) = 0;
};

#endif
