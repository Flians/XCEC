#ifndef _CEC_H_
#define _CEC_H_

#include "libhead.h"
#include "Z3Prover.h"
#include "STPProver.h"
#include "BoolectorProver.h"

class cec
{
private:
    // ofstream fout;

    // assign values to the i-th element in the PIs
    bool assign_PIs_value(vector<Node *> *PIs, size_t i);
    // Calculate all nodes according to PIs
    bool evaluate(vector<Node *> nodes);
public:
    cec();
    cec(const string &path_output);
    ~cec();
    void print_PIs_value(vector<Node *> *PIs, ofstream &output);
    void print_PIs_value(vector<Node *> *PIs, FILE *fout);
    // evaluate from PIs to POs
    void evaluate_from_PIs_to_POs(vector<Node *> *PIs);
    // evaluate from POs to PIs
    void evaluate_from_POs_to_PIs(vector<Node *> *POs);
    // evaluate using z3, timeout is second
    void evaluate_by_z3(vector<vector<Node *> > &layers, int timeout, int max_conflicts, bool is_incremental);
    // evaluate using stp, timeout is second
    void evaluate_by_stp(vector<vector<Node *> > &layers, int timeout, int max_conflicts, bool is_incremental);
    // evaluate using boolector, timeout is second
    void evaluate_by_boolector(vector<vector<Node *> > &layers, int timeout, int max_conflicts, bool is_incremental);
};

#endif