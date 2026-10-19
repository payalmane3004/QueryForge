#ifndef FILESTORAGEENGINE_H
#define FILESTORAGEENGINE_H

#include "storage/StorageEngine.h"
#include "core/Database.h"

class FileStorageEngine : public StorageEngine {
public:
    void save(const Database& database, const string& filename) override;
    void load(Database& database, const string& filename) override;
};

#endif
