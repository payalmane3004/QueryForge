#include "core/Database.h"
#include <fstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

Database::Database() {}

void Database::createTable(const string& name, const vector<Column>& columns) {
    if (hasTable(name)) throw invalid_argument("Table already exists: " + name);
    tables.emplace(name, Table(name, columns));
}

Table& Database::getTable(const string& name) {
    if (!hasTable(name)) throw out_of_range("Table not found: " + name);
    return tables.at(name);
}

const Table& Database::getTable(const string& name) const {
    if (!hasTable(name)) throw out_of_range("Table not found: " + name);
    return tables.at(name);
}

bool Database::hasTable(const string& name) const {
    return tables.find(name) != tables.end();
}

vector<string> Database::getTableNames() const {
    vector<string> names;
    for (const auto& table : tables) names.push_back(table.first);
    return names;
}

int Database::getTableCount() const {
    return static_cast<int>(tables.size());
}

void Database::saveToFile(const string& filename) const {
    ofstream file(filename.c_str());
    if (!file) throw runtime_error("Could not save database file.");

    file << tables.size() << '\n';

    for (const string& name : getTableNames()) {
        const Table& table = getTable(name);
        vector<Column> columns = table.getColumns();
        vector<Record> records = table.getRecords();

        file << quoted(name) << ' ' << columns.size() << '\n';

        for (const Column& column : columns) {
            file << quoted(column.getName()) << ' '
                 << static_cast<int>(column.getDataType()) << '\n';
        }

        file << records.size() << '\n';

        for (const Record& record : records) {
            for (const Value& value : record.getValues()) {
                file << static_cast<int>(value.getType()) << ' '
                     << quoted(value.toString()) << '\n';
            }
        }
    }
}

void Database::loadFromFile(const string& filename) {
    ifstream file(filename.c_str());
    if (!file) return;

    size_t tableCount;
    if (!(file >> tableCount)) return;

    tables.clear();

    for (size_t t = 0; t < tableCount; ++t) {
        string name;
        size_t columnCount;
        file >> quoted(name) >> columnCount;

        vector<Column> columns;

        for (size_t c = 0; c < columnCount; ++c) {
            string columnName;
            int type;
            file >> quoted(columnName) >> type;
            columns.push_back(Column(columnName,
                static_cast<Column::DataType>(type)));
        }

        createTable(name, columns);

        size_t recordCount;
        file >> recordCount;

        for (size_t r = 0; r < recordCount; ++r) {
            vector<Value> values;

            for (size_t c = 0; c < columnCount; ++c) {
                int type;
                string text;
                file >> type >> quoted(text);

                if (type == static_cast<int>(Column::DataType::Integer))
                    values.push_back(Value(stoi(text)));
                else if (type == static_cast<int>(Column::DataType::Float))
                    values.push_back(Value(stof(text)));
                else
                    values.push_back(Value(text));
            }

            getTable(name).insertRecord(Record(values));
        }
    }

    if (!file && !file.eof()) {
        throw runtime_error("Database file is invalid.");
    }
}
