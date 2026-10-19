#include "core/Value.h"

using namespace std;

// Constructor for integer values
Value::Value(int value)
    : type(Column::DataType::Integer),
      intValue(value),
      floatValue(0.0f),
      textValue("") {
}

// Constructor for floating-point values
Value::Value(float value)
    : type(Column::DataType::Float),
      intValue(0),
      floatValue(value),
      textValue("") {
}

// Constructor for text values
Value::Value(const string& value)
    : type(Column::DataType::Text),
      intValue(0),
      floatValue(0.0f),
      textValue(value) {
}

// Returns the data type
Column::DataType Value::getType() const {
    return type;
}

// Converts the value to a string
string Value::toString() const {
    switch (type) {
        case Column::DataType::Integer:
            return to_string(intValue);

        case Column::DataType::Float:
            return to_string(floatValue);

        case Column::DataType::Text:
            return textValue;
    }

    return "";
}