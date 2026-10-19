
#include "core/Column.h"

using namespace std;

// Constructor
Column::Column(const string& name, DataType dataType)
    : name(name), dataType(dataType) {
}

// Returns the column name
string Column::getName() const {
    return name;
}

// Returns the column data type
Column::DataType Column::getDataType() const {
    return dataType;
}

