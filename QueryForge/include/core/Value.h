#ifndef VALUE_H
#define VALUE_H

#include <string>
#include "core/Column.h"

using namespace std;

class Value {
public:
    Value(int value);
    Value(float value);
    Value(const string& value);

    Column::DataType getType() const;
    string toString() const;

private:
    Column::DataType type;

    int intValue;
    float floatValue;
    string textValue;
};

#endif