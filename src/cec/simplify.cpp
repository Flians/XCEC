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
                    std::cerr << "There are some wrong in" << pi->name << std::endl;
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
        std::cout << "PIs is empty in simplify.id_reassign." << endl;
        return;
    }
    map<node *, bool> visit;
    queue<node *> bfs_record;
    int i = 0;
    for (auto pi : (*PIs))
    {
        visit[pi] = true;
        pi->id = i++;
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

vector<vector<node *> *> *simplify::layer_assignment(vector<node *> *PIs, vector<node *> *POs)
{
    vector<vector<node *> *> *layers = new vector<vector<node *> *>;
    if (PIs->empty())
    {
        std::cout << "PIs is empty in simplify.layer_assignment." << std::endl;
        return layers;
    }
    vector<int> visit(init_id, 0);
    vector<int> logic_depth(init_id, 0);
    layers->push_back(PIs);
    int i = 0;
    int nums = PIs->size();
    // layer assignment, and calculate the logic depth of each node
    while (i < layers->size())
    {
        vector<node *> *layer = new vector<node *>;
        for (int j = 0; j < layers->at(i)->size(); j++)
        {
            if (layers->at(i)->at(j)->outs)
            {
                for (auto &out : (*layers->at(i)->at(j)->outs))
                {
                    visit[out->id]++;
                    logic_depth[out->id] = max(logic_depth[layers->at(i)->at(j)->id] + 1, logic_depth[out->id]);
                    if (out->ins->size() == visit[out->id] && out->cell != _EXOR)
                        layer->push_back(out);
                }
            }
        }
        if (!layer->empty())
        {
            layers->push_back(layer);
            nums += layer->size();
        }
        i++;
    }
    layers->push_back(POs);
    nums += POs->size();
    vector<int>().swap(visit);
    vector<int>().swap(logic_depth);
    std::cout << "The layer assignment is over!" << std::endl;
    return layers;
}

void simplify::deduplicate(int i, node *keep, node *dupl, vector<vector<node *> *> *layers)
{
    if (keep->id == dupl->id)
    {
        cerr << "keep is the same as dupl in cec.deduplicate!" << endl;
        exit(-1);
    }
    if (!dupl->outs)
    {
        cerr << "The inputs is empty! in cec.deduplicate!" << endl;
        exit(-1);
    }
    for (auto &out : (*dupl->outs))
    {
        // grandson.ins.push(son)
        vector<node *>::iterator temp_in = out->ins->begin();
        vector<node *>::iterator temp_in_end = out->ins->end();
        while (temp_in != temp_in_end)
        {
            if (dupl == (*temp_in))
            {
                (*temp_in) = keep;
                break;
            }
            temp_in++;
        }
        if (temp_in != temp_in_end)
        {
            // son.outs.push(grandson)
            keep->outs->push_back(out);
        } else {
            cout << "dupl is not equal with keep in simplify.deduplicate." << endl;
        }
    }
    vector<node *>::iterator it = find(layers->at(i)->begin(), layers->at(i)->end(), dupl);
    // layers->at(i)->erase(it);
    *it = *(layers->at(i)->end() - 1);
    layers->at(i)->resize(layers->at(i)->size() - 1);
    vector<node *>().swap(*(dupl->outs));
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
        for (auto &node : (*layers->at(i)))
        {
            level[node->id] = i;
        }
    }
    int reduce = 0;
    for (int i = 0; i < layers->size() - 2; i++)
    {
        map<Gtype, vector<node *>> record;
        for (auto &item : (*layers->at(i)))
        {
            if (item->outs && item->outs->size() > 0)
            {
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
                        record[item->outs->at(j)->cell] = nodes;
                        // record.insert(make_pair(item->outs->at(j)->cell, nodes));
                    }
                }
            }
        }
        for (auto &it : record)
        {
            if (it.second.size() > 1)
            {
                // remove duplicate elements in vector
                sort(it.second.begin(), it.second.end(), [](const node *A, const node *B) {
                    if (A->outs)
                    {
                        if (B->outs)
                        {
                            return A->outs->size() == B->outs->size() ? A->id < B->id : A->outs->size() > B->outs->size();
                        }
                        return true;
                    }
                    else
                    {
                        if (B->outs)
                        {
                            return false;
                        }
                        return A->id < B->id;
                    }
                });
                it.second.erase(unique(it.second.begin(), it.second.end()), it.second.end());
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
                        int candidate_size = it.second.size();
                        for (int ri = si + 1; ri < candidate_size; ri++)
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
                                    // it.second.erase(it.second.begin() + ri);
                                    --candidate_size;
                                    *(it.second.begin() + ri) = *(it.second.begin() + candidate_size);
                                    --ri;
                                }
                            }
                        }
                        if (it.second.size() > candidate_size)
                        {
                            reduce += it.second.size() - candidate_size;
                            it.second.resize(candidate_size);
                        }
                    }
                }
            }
            it.second.clear();
        }
        record.clear();
    }
    for (int i = 0; i < layers->size(); i++)
    {
        if (layers->at(i)->empty())
        {
            layers->erase(layers->begin() + i);
            --i;
        }
    }
    vector<int>().swap(level);
    std::cout << "The number of INV, BUF, and others reduction is " << reduce << std::endl;
}