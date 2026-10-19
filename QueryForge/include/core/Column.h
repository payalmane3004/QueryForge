
#ifndef COLUMN_H
#define COLUMN_H

#include <string>
using namespace std;

class Column {
public:
    enum class DataType {
        Integer,
        Text,
        Float
    };

    Column(const string& name, DataType dataType);

    string getName() const;
    DataType getDataType() const;

private:
    string name;
    DataType dataType;
};

#endif


