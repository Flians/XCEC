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
    cout << "The layer assignment is over!" << endl;
    return layers;
}

void simplify::deduplicate(int i, node *keep, node *dupl, vector<vector<node *> *> *layers)
{
    if (!dupl->outs)
    {
        cerr << "The inputs is empty! in cec.deduplicate!" << endl;
        exit(-1);
    }
    for (auto &out : (*dupl->outs))
    {
        // grandson.ins.push(son)
        out->ins->push_back(keep);
        // son.outs.push(grandson)
        keep->outs->push_back(out);
    }
    layers->at(i)->erase(find(layers->at(i)->begin(), layers->at(i)->end(), dupl));
    delete dupl;
}

void simplify::reduce_repeat_nodes(vector<vector<node *> *> *layers)
{
    if (!layers || layers->size() == 0)
    {
        cerr << "The layers is empty in simplify.reduce_repeat_nodes!" << endl;
        exit(-1);
    }
    vector<int> level(init_id, 0);
    for (int i = 0; i < layers->size(); i++)
    {
        for (auto &node : (*layers->at(i))) {
            level[node->id] = i;
        }
    }
    int reduce = 0;
    for (int i = 0; i < layers->size() - 2; i++)
    {
        for (auto &item : (*layers->at(i)))
        {
            if (item->outs && item->outs->size() > 0)
            {
                map<Gtype, vector<node *>> record;
                for (int j = 0; j < item->outs->size(); j++)
                {
                    if (record.count(item->outs->at(j)->cell))
                    {
                        record[item->outs->at(j)->cell].push_back(item->outs->at(j));
                    }
                    else
                    {
                        vector<node *> nodes;
                        nodes.push_back(item->outs->at(j));
                        record.insert(make_pair(item->outs->at(j)->cell, nodes));
                    }
                }
                for (auto &it : record)
                {
                    if (it.second.size() > 1)
                    {
                        sort(it.second.begin(), it.second.end());
                        if (it.first == BUF || it.first == INV)
                        {
                            for (int d = 1; d < it.second.size(); ++d)
                            {
                                this->deduplicate(level[it.second.at(d)->id], it.second.at(0), it.second.at(d), layers);
                                reduce++;
                            }
                        }
                        else
                        {
                            for (int si = 0; si < it.second.size(); si++)
                            {
                                for (int ri = si + 1; ri < it.second.size(); ri++)
                                {
                                    if (it.second.at(si)->ins->size() == it.second.at(ri)->ins->size())
                                    {
                                        bool flag = true;
                                        for (int ii = 0; ii < it.second.at(si)->ins->size(); ii++)
                                        {
                                            if (it.second.at(si)->ins->at(ii)->id != it.second.at(ri)->ins->at(ii)->id)
                                            {
                                                flag = false;
                                                break;
                                            }
                                        }
                                        if (flag)
                                        {
                                            this->deduplicate(level[it.second.at(ri)->id], it.second.at(si), it.second.at(ri), layers);
                                            it.second.erase(it.second.begin() + ri);
                                            reduce++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    it.second.clear();
                }
                record.clear();
            }
        }
        if (layers->at(i)->empty()) {
            layers->erase(layers->begin()+i);
            i--;
        }
    }
    vector<int>().swap(level);
    cout << "The number of INV, BUF, and others reduction is " << reduce << endl;
}