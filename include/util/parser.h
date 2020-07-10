#ifndef _PARSER_H_
#define _PARSER_H_

#include "libhead.h"

class parser
{
private:
    vector<node *> wires_golden;
    vector<node *> wires_revised;

public:
    vector<node *> PIs;
    vector<node *> POs;
    vector<node *> constants;
    
    parser(/* args */);
    ~parser();
    vector<node *> &get_PIs();
    vector<node *> &get_POs();
    vector<node *> &get_constants();

    // find the node from vector<node *> *nodes by name
    node *find_node_by_name(vector<node *> &nodes, string &name);

    // replace the node from vector<node *> *nodes with the new_node, which the name of this node is same as the new_node
    bool replace_node_by_name(vector<node *> &nodes, node *new_node);

    // parse the verilog file
    void parse_verilog(stringstream &in);

    // parse the revised verilog file
    void parse_revised(stringstream &in);

    // build the miter for the Combinational Equivalence Checking (CEC)
    // POs is PIs_golden, and return POs
    void build_miter(vector<node *> &PIs_golden, vector<node *> &POs_golden, vector<node *> &PIs_revised, vector<node *> &POs_revised);

    // parse the verilog files, and return the PIs and POs of the miter
    void parse(ifstream &golden, ifstream &revised);

    // parse the verilog files, and return the PIs and POs of the miter
    void parse(const string &path_golden, const string &path_revised);

    // print the graph
    void printG(vector<node *> *);
};

#endif
