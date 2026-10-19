
#include "core/Record.h"

using namespace std;

// Default constructor
Record::Record() {
}

// Constructor with values
Record::Record(const vector<Value>& values) : values(values) {
}

// Adds a value to the record
void Record::addValue(const Value& value) {
    values.push_back(value);
}

// Returns a value at the specified index
Value Record::getValue(int index) const {
    return values.at(index);
}

// Returns the number of values
int Record::getSize() const {
    return static_cast<int>(values.size());
}

// Returns all values
vector<Value> Record::getValues() const {
    return values;
}

