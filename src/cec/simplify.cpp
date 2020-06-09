#include "simplify.h"

simplify::simplify(/* args */)
{
}

simplify::~simplify()
{
}

bool simplify::replace_node_by_id(vector<node *> *nodes, node *new_node, int id)
{
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

/**
 * clean all wires and bufs
 * from PIs to POs
 */
void simplify::clean_wire_buf(vector<node *> *miter)
{
    if (!miter || miter->size() == 0)
        return;
    int len = miter->size();
    for (int i = 0; i < len; ++i)
    {
        node *pi = miter->at(i);
        if (!(pi->outs) || pi->outs->size() == 0)
        {
            continue;
        }

        if (pi->cell == WIRE || pi->cell == BUF)
        {
            if (pi->ins->size() != 1)
            {
                cerr << pi->name << " WIRE have none or more one inputs in simplify.clean_wire_buf!" << endl;
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
                    if (pi == (*temp_in))
                    {
                        (*temp_in) = tin;
                        break;
                    }
                    temp_in++;
                }
                if (temp_in != temp_in_end)
                {
                    tin->outs->push_back(*it);
                }
                else
                {
                    cerr << "There are some wrong in" << pi->name << endl;
                    exit(-1);
                }
                ++it;
            }
            vector<node *>().swap(*(pi->outs));
            delete pi;
            clean_wire_buf(tin->outs);
        }
        // this node has been visited.
        else if (pi->cell != IN && pi->outs->at(0)->cell != WIRE)
        {
            continue;
        }
        else
        {
            clean_wire_buf(pi->outs);
        }
    }
}

void simplify::id_reassign(vector<node *> *PIs)
{
    if (PIs->empty())
    {
        cout << "PIs is empty in simplify.id_reassign" << endl;
        return;
    }
    map<node *, bool> visit;
    queue<node *> bfs_record;
    int i = 0;
    for (auto pi : (*PIs))
    {
        visit[pi] = true;
        if (pi->name.find("clk") != string::npos && !bfs_record.empty())
        {
            pi->id = 0;
            bfs_record.front()->id = i++;
            swap(PIs->at(0), pi);
        }
        else
        {
            pi->id = i++;
        }
        bfs_record.push(pi);
    }
    while (!bfs_record.empty())
    {
        node *item = bfs_record.front();
        if (item->outs)
        {
            for (auto out : (*item->outs))
            {
                if (visit.count(out) == 0)
                {
                    visit[out] = true;
                    out->id = i++;
                    bfs_record.push(out);
                }
            }
        }
        bfs_record.pop();
    }
    init_id = i;
    visit.clear();
}


vector<vector<node *> *> *simplify::layer_assignment(vector<node *> *PIs)
{
    vector<vector<node *> *> *layers = new vector<vector<node *> *>;
    if (PIs->empty())
    {
        cout << "PIs is empty in simplify.layer_assignment" << endl;
        return layers;
    }
    this->id_reassign(PIs);
    layers->push_back(PIs);
    int i = 0;
    vector<int> visit(init_id, 0);
    vector<int> logic_depth(init_id, 0);
    // layer assignment, and calculate the logic depth of each node
    while (i < layers->size())
    {
        vector<node *> *layer = new vector<node *>;
        for (int j = 0; j < layers->at(i)->size(); j++)
        {
            if (layers->at(i)->at(j)->outs)
            {
                for (auto &out:(*layers->at(i)->at(j)->outs)) {
                    visit[out->id]++;
                    logic_depth[out->id] = max(logic_depth[layers->at(i)->at(j)->id] + 1, logic_depth[out->id]);
                    if (out->ins->size() == visit[out->id])
                        layer->push_back(out);
                }
            }
        }
        if (!layer->empty())
        {
            layers->push_back(layer);
        }
        i++;
    }
    vector<int>().swap(visit);
    vector<int>().swap(logic_depth);
    return layers;
}