#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <vector>
#include "core/Column.h"
#include "core/Record.h"

using namespace std;

class Table {
public:
    Table(const string& name, const vector<Column>& columns);

    string getName() const;
    vector<Column> getColumns() const;
    vector<Record> getRecords() const;

    void insertRecord(const Record& record);
    int getRecordCount() const;

    int updateRecords(int columnIndex, const Value& newValue,
                      int conditionColumn, const Value& conditionValue,
                      const string& op);

    int deleteRecords(int conditionColumn, const Value& conditionValue,
                      const string& op);

private:
    string name;
    vector<Column> columns;
    vector<Record> records;

    bool matches(const Record& record, int columnIndex,
                 const Value& value, const string& op) const;
};

#endif