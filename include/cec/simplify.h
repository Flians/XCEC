#ifndef _SIMPLIFY_H_
#define _SIMPLIFY_H_

#include "libhead.h"
#include "roaring.hh"

class simplify
{
private:
    vector<vector<Node *> > layers;

public:
    simplify(/* args */);
    ~simplify();
    vector<vector<Node *> > &get_layers();

    // reassign id of each node, and layer assigment according to the logic depth, and achieve path balancing
    vector<vector<Node *> > &id_reassign_and_layered(vector<Node *> &PIs, vector<Node *> &POs);

    // reassign id of each node
    void id_reassign();

    // merge the equal nodes
    int merge_nodes_between_networks();
};

#endif