#ifndef _CEC_H_
#define _CEC_H_

#include "libhead.h"

class cec
{
private:
    ofstream fout;
public:
    cec();
    cec(const string &path_output);
    ~cec();
    void print_PIs_value(vector<node *> *PIs, ofstream &output);
    // assign values to the i-th element in the PIs
    bool assign_PIs_value(vector<node *> *PIs, int i);
    bool evaluate(vector<node *> nodes);
    void evaluate(vector<node *> *PIs);
};

#endif