#include "core/Table.h"
#include <stdexcept>

using namespace std;

Table::Table(const string& name, const vector<Column>& columns)
    : name(name), columns(columns) {}

string Table::getName() const {
    return name;
}

vector<Column> Table::getColumns() const {
    return columns;
}

vector<Record> Table::getRecords() const {
    return records;
}

void Table::insertRecord(const Record& record) {
    if (record.getSize() != static_cast<int>(columns.size())) {
        throw invalid_argument("Record size does not match table columns.");
    }

    vector<Value> values = record.getValues();

    for (size_t i = 0; i < columns.size(); ++i) {
        if (values[i].getType() != columns[i].getDataType()) {
            throw invalid_argument(
                "Data type mismatch for column: " + columns[i].getName()
            );
        }
    }

    records.push_back(record);
}

int Table::getRecordCount() const {
    return static_cast<int>(records.size());
}

bool Table::matches(const Record& record, int columnIndex,
                    const Value& value, const string& op) const {
    Value current = record.getValue(columnIndex);

    if (current.getType() != value.getType()) {
        throw invalid_argument("Comparison value type mismatch.");
    }

    if (current.getType() == Column::DataType::Text) {
        string a = current.toString();
        string b = value.toString();

        if (op == "=" || op == "==") return a == b;
        if (op == "!=" || op == "<>") return a != b;

        throw invalid_argument(
            "Text comparisons support only =, ==, !=, and <>."
        );
    }

    double a = stod(current.toString());
    double b = stod(value.toString());

    if (op == "=" || op == "==") return a == b;
    if (op == "!=" || op == "<>") return a != b;
    if (op == ">") return a > b;
    if (op == "<") return a < b;
    if (op == ">=") return a >= b;
    if (op == "<=") return a <= b;

    throw invalid_argument("Unsupported comparison operator.");
}

int Table::updateRecords(int columnIndex, const Value& newValue,
                         int conditionColumn, const Value& conditionValue,
                         const string& op) {
    if (columnIndex < 0 ||
        columnIndex >= static_cast<int>(columns.size())) {
        throw out_of_range("Invalid column index.");
    }

    if (conditionColumn < 0 ||
        conditionColumn >= static_cast<int>(columns.size())) {
        throw out_of_range("Invalid condition column.");
    }

    if (newValue.getType() != columns[columnIndex].getDataType()) {
        throw invalid_argument("UPDATE value type does not match column.");
    }

    int updated = 0;

    for (Record& record : records) {
        if (matches(record, conditionColumn, conditionValue, op)) {
            vector<Value> values = record.getValues();
            values[columnIndex] = newValue;
            record = Record(values);
            ++updated;
        }
    }

    return updated;
}

int Table::deleteRecords(int conditionColumn,
                         const Value& conditionValue,
                         const string& op) {
    if (conditionColumn < 0 ||
        conditionColumn >= static_cast<int>(columns.size())) {
        throw out_of_range("Invalid condition column.");
    }

    int deleted = 0;

    for (vector<Record>::iterator it = records.begin();
         it != records.end();) {
        if (matches(*it, conditionColumn, conditionValue, op)) {
            it = records.erase(it);
            ++deleted;
        } else {
            ++it;
        }
    }

    return deleted;
}
