#include "simplify.h"

simplify::simplify(/* args */)
{
}

simplify::~simplify()
{
}

bool simplify::replace_node_by_id(vector<node *> *nodes, node *new_node, int id){
    for (auto &node : *nodes)
    {
        if (id == node->id)
        {
            node = new_node;
            return true;
        }
    }
    return false;
}

void simplify::clean_wire_buf(vector<node *> *miter) {
    if (!miter || miter->size() == 0)
        return;
    int len = miter->size();
    for (int i=0; i < len; ++i)
    {
        node *pi = miter->at(i);
        cout << pi->name << " "<< Str_Value[pi->cell] << endl;
        // clean_wire_buf(pi->outs);

        if (!(pi->outs)) {
            continue;
        }
        
        if (pi->cell == WIRE) {
            if (pi->ins->size()!=1) {
                perror((pi->name + " WIRE have none or more one inputs in simplify.clean_wire_buf!").c_str());
                exit(-1);
            }
            node *tin = pi->ins->at(0);
            vector<node *>::iterator it = pi->outs->begin();
            vector<node *>::iterator it_end = pi->outs->end();
            while (it != it_end)
            {
                vector<node *>::iterator temp_in = (*it)->ins->begin();
                vector<node *>::iterator temp_in_end = (*it)->ins->end();
                while (temp_in != temp_in_end)
                {
                    if (pi == (*temp_in)) {
                        (*temp_in) = tin;
                        break;
                    }
                    temp_in++;
                }
                if (temp_in != temp_in_end) {
                    tin->outs->push_back(*it);
                } else
                {
                    perror(("There are some wrong in" + pi->name).c_str());
                    exit(-1);
                }
                ++it;
            }
            vector<node *>().swap(*(pi->outs));
            delete pi;
            clean_wire_buf(tin->outs);
        } else
        {
            clean_wire_buf(pi->outs);
        }
    }
}