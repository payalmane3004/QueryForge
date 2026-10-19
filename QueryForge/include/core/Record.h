
#ifndef RECORD_H
#define RECORD_H

#include <vector>
#include "core/Value.h"

using namespace std;

class Record {
public:
    Record();
    Record(const vector<Value>& values);

    void addValue(const Value& value);
    Value getValue(int index) const;
    int getSize() const;
    vector<Value> getValues() const;

private:
    vector<Value> values;
};

#endif

