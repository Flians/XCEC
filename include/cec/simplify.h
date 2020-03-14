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

    void clean_wire_buf(vector<node *> *);
};

#endif