#ifndef _SIMPLIFY_H_
#define _SIMPLIFY_H_

#include "libhead.h"

class simplify
{
private:
    /* data */
public:
    simplify(/* args */);
    ~simplify();

    // replace the node from vector<node *> *nodes with the new_node, which the id of this node is id
    bool replace_node_by_id(vector<node *> *nodes, node *new_node, int id);

    // clean all wires and bufs from PIs to POs
    void clean_wire_buf(vector<node *> *);

    // reassign id of each node
    void id_reassign(vector<node *> *PIs);

    // layer assigment according to the logic depth, and achieve path balancing
    vector<vector<node *> *> *layer_assignment(vector<node *> *PIs, vector<node *> *POs);
    
    // delete duplicate node, i is the index level of dupl
    void deduplicate(int i, node *keep, node *dupl, vector<vector<node *> *> *layers);

    // reduce the number of INV, BUF and others
    void reduce_repeat_nodes(vector<vector<node *> *> *layers);
};

#endif