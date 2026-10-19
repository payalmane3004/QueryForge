#include "execution/QueryExecutor.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

using namespace std;

namespace {
    string trim(const string& text) {
        size_t first = text.find_first_not_of(" \t\r\n");

        if (first == string::npos) {
            return "";
        }

        size_t last = text.find_last_not_of(" \t\r\n");
        return text.substr(first, last - first + 1);
    }

    string uppercase(string text) {
        transform(text.begin(), text.end(), text.begin(),
            [](unsigned char c) {
                return static_cast<char>(toupper(c));
            });

        return text;
    }

   
Value parseUpdateValue(const string& text) {
    string value = text;

    size_t first = value.find_first_not_of(" \t\r\n");
    size_t last = value.find_last_not_of(" \t\r\n");

    if (first == string::npos) {
        throw invalid_argument("Missing value.");
    }

    value = value.substr(first, last - first + 1);

    if (value.size() >= 2 &&
        ((value.front() == '\'' && value.back() == '\'') ||
         (value.front() == '"' && value.back() == '"'))) {
        return Value(value.substr(1, value.size() - 2));
    }

    try {
        size_t used = 0;

        if (value.find('.') != string::npos) {
            float number = stof(value, &used);

            if (used != value.size()) {
                throw invalid_argument("Invalid number.");
            }

            return Value(number);
        }

        int number = stoi(value, &used);

        if (used != value.size()) {
            throw invalid_argument("Invalid number.");
        }

        return Value(number);
    } catch (...) {
        throw invalid_argument("Invalid value: " + value);
    }
}

void parseCondition(const string& condition,
                    string& columnName,
                    string& op,
                    string& valueText) {
    const string operators[] = {
        ">=", "<=", "!=", "<>", "==", "=", ">", "<"
    };

    for (const string& candidate : operators) {
        size_t pos = condition.find(candidate);

        if (pos != string::npos) {
            columnName = trim(condition.substr(0, pos));
            op = candidate;
            valueText = trim(
                condition.substr(pos + candidate.size())
            );

            if (columnName.empty() || valueText.empty()) {
                throw invalid_argument("Invalid condition.");
            }

            return;
        }
    }

    throw invalid_argument("Invalid condition.");
}
    bool compareValues(const Value& left,
                       const Value& right,
                       const string& op) {
        if (left.getType() != right.getType()) {
            throw invalid_argument(
                "WHERE value type does not match column type."
            );
        }

        if (left.getType() == Column::DataType::Integer) {
            int a = stoi(left.toString());
            int b = stoi(right.toString());

            if (op == "=" || op == "==") return a == b;
            if (op == "!=" || op == "<>") return a != b;
            if (op == ">") return a > b;
            if (op == "<") return a < b;
            if (op == ">=") return a >= b;
            if (op == "<=") return a <= b;
        }
        else if (left.getType() == Column::DataType::Float) {
            float a = stof(left.toString());
            float b = stof(right.toString());

            if (op == "=" || op == "==") return a == b;
            if (op == "!=" || op == "<>") return a != b;
            if (op == ">") return a > b;
            if (op == "<") return a < b;
            if (op == ">=") return a >= b;
            if (op == "<=") return a <= b;
        }
        else {
            string a = left.toString();
            string b = right.toString();

            if (op == "=" || op == "==") return a == b;
            if (op == "!=" || op == "<>") return a != b;

            throw invalid_argument(
                "Text columns support only =, ==, !=, and <>."
            );
        }

        throw invalid_argument("Unsupported comparison operator.");
    }

    Value parseValue(string text) {
        text = trim(text);

        if (text.size() >= 2 &&
            ((text.front() == '\'' && text.back() == '\'') ||
             (text.front() == '"' && text.back() == '"'))) {
            return Value(text.substr(1, text.size() - 2));
        }

        try {
            size_t used = 0;

            if (text.find('.') != string::npos) {
                float value = stof(text, &used);

                if (used != text.size()) {
                    throw invalid_argument("Invalid numeric value.");
                }

                return Value(value);
            }

            int value = stoi(text, &used);

            if (used != text.size()) {
                throw invalid_argument("Invalid numeric value.");
            }

            return Value(value);
        }
        catch (...) {
            throw invalid_argument("Invalid value: " + text);
        }
    }

    Value matchColumnType(const Value& value, Column::DataType type) {
        if (type == Column::DataType::Float &&
            value.getType() == Column::DataType::Integer) {
            return Value(static_cast<float>(stoi(value.toString())));
        }
        return value;
    }
}

QueryExecutor::QueryExecutor(Database& database)
    : database(database) {
}

void QueryExecutor::execute(const string& query) {
    string command = trim(query);

    if (!command.empty() && command.back() == ';') {
        command.pop_back();
        command = trim(command);
    }

    string normalized = uppercase(command);

    if (normalized.find("CREATE TABLE ") == 0) {
        executeCreate(command);
    }
    else if (normalized.find("INSERT INTO ") == 0) {
        executeInsert(command);
    }
    else if (normalized.find("SELECT ") == 0) {
        executeSelect(command);
    }
    else if (normalized.find("UPDATE ") == 0) {
    executeUpdate(command);
}
else if (normalized.find("DELETE FROM ") == 0) {
    executeDelete(command);
}
    else {
        throw invalid_argument("Unsupported or invalid query.");
    }
}

void QueryExecutor::executeCreate(const string& query) {
    size_t start = 13;
    size_t open = query.find('(', start);
    size_t close = query.rfind(')');

    if (open == string::npos ||
        close == string::npos ||
        close <= open) {
        throw invalid_argument("Invalid CREATE TABLE syntax.");
    }

    string tableName = trim(query.substr(start, open - start));

    if (tableName.empty()) {
        throw invalid_argument("Missing table name.");
    }

    string definition = query.substr(open + 1, close - open - 1);
    stringstream stream(definition);
    string item;
    vector<Column> columns;

    while (getline(stream, item, ',')) {
        stringstream columnStream(item);
        string columnName;
        string typeName;

        if (!(columnStream >> columnName >> typeName)) {
            throw invalid_argument("Invalid column definition.");
        }

        string extra;

        if (columnStream >> extra) {
            throw invalid_argument("Invalid column definition: " + item);
        }

        typeName = uppercase(typeName);
        Column::DataType type;

        if (typeName == "INTEGER" || typeName == "INT") {
            type = Column::DataType::Integer;
        }
        else if (typeName == "FLOAT" || typeName == "REAL") {
            type = Column::DataType::Float;
        }
        else if (typeName == "TEXT" || typeName == "STRING") {
            type = Column::DataType::Text;
        }
        else {
            throw invalid_argument("Unsupported column type: " + typeName);
        }

        columns.push_back(Column(columnName, type));
    }

    if (columns.empty()) {
        throw invalid_argument("A table must have at least one column.");
    }

    database.createTable(tableName, columns);

    cout << "Table '" << tableName
         << "' created successfully." << endl;
}

void QueryExecutor::executeInsert(const string& query) {
    size_t start = 12;
    size_t valuesPosition = uppercase(query).find("VALUES", start);

    if (valuesPosition == string::npos) {
        throw invalid_argument("INSERT query must contain VALUES.");
    }

    string tableName = trim(query.substr(start, valuesPosition - start));

    if (tableName.empty()) {
        throw invalid_argument("Missing table name.");
    }

    size_t open = query.find('(', valuesPosition);
    size_t close = query.rfind(')');

    if (open == string::npos ||
        close == string::npos ||
        close <= open) {
        throw invalid_argument("Invalid INSERT syntax.");
    }

    string content = query.substr(open + 1, close - open - 1);
    vector<Value> values;

    string item;
    string current;
    char quote = '\0';

    // Split values without splitting commas inside quoted text.
    for (size_t i = 0; i < content.size(); ++i) {
        char c = content[i];

        if ((c == '\'' || c == '"') &&
            (quote == '\0' || quote == c)) {
            quote = (quote == '\0') ? c : '\0';
            current += c;
        }
        else if (c == ',' && quote == '\0') {
            values.push_back(parseValue(current));
            current.clear();
        }
        else {
            current += c;
        }
    }

    if (quote != '\0') {
        throw invalid_argument("Unclosed quote in INSERT.");
    }

    if (!trim(current).empty()) {
        values.push_back(parseValue(current));
    }

    Table& table = database.getTable(tableName);
    vector<Column> columns = table.getColumns();

    if (values.size() != columns.size()) {
        throw invalid_argument(
            "Number of values does not match columns."
        );
    }

    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i].getType() != columns[i].getDataType()) {
            throw invalid_argument(
                "Data type mismatch for column: " +
                columns[i].getName()
            );
        }
    }

    table.insertRecord(Record(values));

    cout << "Record inserted successfully." << endl;
}

void QueryExecutor::executeSelect(const string& query) {
    string normalized = uppercase(query);

    size_t from = normalized.find(" FROM ");

    if (from == string::npos) {
        throw invalid_argument("SELECT query must contain FROM.");
    }

    string selection = trim(query.substr(7, from - 7));
    string remainder = trim(query.substr(from + 6));

    if (selection != "*") {
        throw invalid_argument("Currently, SELECT supports * only.");
    }

    string remainderUpper = uppercase(remainder);
    size_t wherePosition = remainderUpper.find(" WHERE ");

    string tableName;
    string condition;

    if (wherePosition == string::npos) {
        tableName = trim(remainder);
    }
    else {
        tableName = trim(remainder.substr(0, wherePosition));
        condition = trim(remainder.substr(wherePosition + 7));
    }

    if (tableName.empty()) {
        throw invalid_argument("Missing table name.");
    }

    Table& table = database.getTable(tableName);
    vector<Column> columns = table.getColumns();
    vector<Record> records = table.getRecords();

    int filterColumn = -1;
    string op;
    // Parse WHERE condition: column operator value.
    if (!condition.empty()) {
        string operators[] = {">=", "<=", "!=", "<>", "==", "=", ">", "<"};
        size_t operatorPosition = string::npos;

        for (const string& candidate : operators) {
            size_t pos = condition.find(candidate);

            if (pos != string::npos) {
                operatorPosition = pos;
                op = candidate;
                break;
            }
        }

        if (operatorPosition == string::npos) {
            throw invalid_argument(
                "Invalid WHERE condition. Example: WHERE id = 1"
            );
        }

        string columnName = trim(condition.substr(0, operatorPosition));
        string valueText = trim(
            condition.substr(operatorPosition + op.size())
        );

        if (columnName.empty() || valueText.empty()) {
            throw invalid_argument("Invalid WHERE condition.");
        }

        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].getName() == columnName) {
                filterColumn = static_cast<int>(i);
                break;
            }
        }

        if (filterColumn == -1) {
            throw invalid_argument("Unknown column: " + columnName);
        }

        Value expected = matchColumnType(
            parseValue(valueText), columns[filterColumn].getDataType()
        );

        if (expected.getType() != columns[filterColumn].getDataType()) {
            throw invalid_argument(
                "WHERE value type does not match column type."
            );
        }

        cout << "\n";

        for (const Column& column : columns) {
            cout << column.getName() << "\t";
        }

        cout << "\n";

        int count = 0;

        for (const Record& record : records) {
            if (compareValues(
                    record.getValue(filterColumn),
                    expected,
                    op)) {
                for (const Value& value : record.getValues()) {
                    cout << value.toString() << "\t";
                }

                cout << "\n";
                ++count;
            }
        }

        cout << count << " record(s) returned." << endl;
        return;
    }

    cout << "\n";

    for (const Column& column : columns) {
        cout << column.getName() << "\t";
    }

    cout << "\n";

    for (const Record& record : records) {
        for (const Value& value : record.getValues()) {
            cout << value.toString() << "\t";
        }

        cout << "\n";
    }

    cout << records.size() << " record(s) returned." << endl;
}

void QueryExecutor::executeUpdate(const string& query) {
    string normalized = uppercase(query);

    size_t setPosition = normalized.find(" SET ");
    size_t wherePosition = normalized.find(" WHERE ");

    if (setPosition == string::npos ||
        wherePosition == string::npos ||
        wherePosition <= setPosition) {
        throw invalid_argument(
            "Syntax: UPDATE table SET column = value WHERE column = value"
        );
    }

    string tableName = trim(
        query.substr(7, setPosition - 7)
    );

    string assignment = trim(
        query.substr(setPosition + 5,
                     wherePosition - setPosition - 5)
    );

    string condition = trim(
        query.substr(wherePosition + 7)
    );

    size_t equalsPosition = assignment.find('=');

    if (equalsPosition == string::npos) {
        throw invalid_argument("Invalid UPDATE assignment.");
    }

    string targetColumn = trim(
        assignment.substr(0, equalsPosition)
    );

    Value newValue = parseUpdateValue(
        assignment.substr(equalsPosition + 1)
    );

    string conditionColumn;
    string op;
    string conditionValueText;

    parseCondition(
        condition, conditionColumn, op, conditionValueText
    );

    Table& table = database.getTable(tableName);
    vector<Column> columns = table.getColumns();

    int targetIndex = -1;
    int conditionIndex = -1;

    for (size_t i = 0; i < columns.size(); ++i) {
        if (columns[i].getName() == targetColumn) {
            targetIndex = static_cast<int>(i);
        }

        if (columns[i].getName() == conditionColumn) {
            conditionIndex = static_cast<int>(i);
        }
    }

    if (targetIndex == -1) {
        throw invalid_argument("Unknown column: " + targetColumn);
    }

    if (conditionIndex == -1) {
        throw invalid_argument("Unknown column: " + conditionColumn);
    }

    if (newValue.getType() != columns[targetIndex].getDataType()) {
        throw invalid_argument("UPDATE value type mismatch.");
    }

    Value conditionValue = matchColumnType(
        parseUpdateValue(conditionValueText),
        columns[conditionIndex].getDataType()
    );

    if (conditionValue.getType() !=
        columns[conditionIndex].getDataType()) {
        throw invalid_argument("WHERE value type mismatch.");
    }

    int count = table.updateRecords(
        targetIndex, newValue,
        conditionIndex, conditionValue, op
    );

    cout << count << " record(s) updated." << endl;
}

void QueryExecutor::executeDelete(const string& query) {
    string normalized = uppercase(query);

    size_t wherePosition = normalized.find(" WHERE ");

    if (wherePosition == string::npos) {
        throw invalid_argument(
            "DELETE requires a WHERE condition to prevent accidental deletion."
        );
    }

    string tableName = trim(
        query.substr(12, wherePosition - 12)
    );

    string condition = trim(
        query.substr(wherePosition + 7)
    );

    if (tableName.empty()) {
        throw invalid_argument("Missing table name.");
    }

    string conditionColumn;
    string op;
    string conditionValueText;

    parseCondition(
        condition, conditionColumn, op, conditionValueText
    );

    Table& table = database.getTable(tableName);
    vector<Column> columns = table.getColumns();

    int conditionIndex = -1;

    for (size_t i = 0; i < columns.size(); ++i) {
        if (columns[i].getName() == conditionColumn) {
            conditionIndex = static_cast<int>(i);
            break;
        }
    }

    if (conditionIndex == -1) {
        throw invalid_argument("Unknown column: " + conditionColumn);
    }

    Value conditionValue = matchColumnType(
        parseUpdateValue(conditionValueText),
        columns[conditionIndex].getDataType()
    );

    if (conditionValue.getType() !=
        columns[conditionIndex].getDataType()) {
        throw invalid_argument("WHERE value type mismatch.");
    }

    int count = table.deleteRecords(
        conditionIndex, conditionValue, op
    );

    cout << count << " record(s) deleted." << endl;
}