#include <iostream>
#include <string>
#include "core/Database.h"
#include "execution/QueryExecutor.h"
#include "storage/FileStorageEngine.h"

using namespace std;

int main() {
    Database database;
    QueryExecutor executor(database);
    FileStorageEngine fileStorage;
    StorageEngine& storage = fileStorage;
    const string file = "data/queryforge.db";

    try {
        storage.load(database, file);
        cout << "QueryForge - Mini Database Engine\n";
        cout << "Commands: CREATE TABLE, INSERT, SELECT, UPDATE, DELETE, EXIT\n";
    } catch (const exception& e) {
        cerr << "Could not load database: " << e.what() << '\n';
        return 1;
    }

    string query;

    while (true) {
        cout << "\nQueryForge> ";
        if (!getline(cin, query)) break;

        if (query == "EXIT" || query == "exit" ||
            query == "QUIT" || query == "quit") {
            cout << "Goodbye!\n";
            break;
        }

        if (query.empty()) continue;

        try {
            executor.execute(query);
            storage.save(database, file);
        } catch (const exception& e) {
            cerr << "Error: " << e.what() << '\n';
        }
    }

    return 0;
}
