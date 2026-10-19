#include "storage/FileStorageEngine.h"

void FileStorageEngine::save(const Database& database, const string& filename) {
    database.saveToFile(filename);
}

void FileStorageEngine::load(Database& database, const string& filename) {
    database.loadFromFile(filename);
}
