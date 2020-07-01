#ifndef _CEC_H_
#define _CEC_H_

#include "libhead.h"

class cec
{
private:
    ofstream fout;

    // assign values to the i-th element in the PIs
    bool assign_PIs_value(vector<node *> *PIs, int i);
    // Calculate all nodes according to PIs
    bool evaluate(vector<node *> nodes);
public:
    cec();
    cec(const string &path_output);
    ~cec();
    void print_PIs_value(vector<node *> *PIs, ofstream &output);
    // evaluate from PIs to POs
    void evaluate_from_PIs_to_POs(vector<node *> *PIs);
    // evaluate from POs to PIs
    void evaluate_from_POs_to_PIs(vector<node *> *POs);
    // evaluate using z3
    void evaluate_by_z3(vector<vector<node *> *> *layers);
    // evaluate using stp
    void evaluate_by_stp(vector<vector<node *> *> *layers);
};

#endif