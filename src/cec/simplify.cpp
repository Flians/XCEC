#include "simplify.h"

simplify::simplify(/* args */)
{
}

simplify::~simplify()
{
    /* free up space */
    for (auto &item : this->layers)
    {
        // Now delete the vector contents in a single  line.
        for_each(item.begin(), item.end(), DeleteVector<node *>());
        //Clear the vector
        vector<node *>().swap(item);
    }
    vector<vector<node *>>().swap(this->layers);
}

vector<vector<node *>> &simplify::get_layers()
{
    return this->layers;
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
            node *tin = pi->ins->front();
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
                    ++temp_in;
                }
                if (temp_in != temp_in_end)
                {
                    tin->outs->emplace_back(*it);
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
        else if (pi->cell != IN && pi->outs->front()->cell != WIRE)
        {
            continue;
        }
        else
        {
            clean_wire_buf(pi->outs);
        }
    }
}

vector<vector<node *>> &simplify::id_reassign_and_layered(vector<node *> *PIs, vector<node *> *POs)
{
    if (PIs->empty())
    {
        std::cout << "PIs is empty in simplify.id_reassign." << endl;
        return this->layers;
    }
    map<node *, int> visit;
    queue<node *> bfs_record;
    // reassign id of each node, and obtain the length of the longest path
    int i = 0;
    for (auto pi : (*PIs))
    {
        visit[pi] = 1;
        pi->id = i++;
        bfs_record.push(pi);
    }
    int longest_path = 0;
    while (!bfs_record.empty())
    {
        node *item = bfs_record.front();
        if (item->outs)
        {
            for (auto out : (*item->outs))
            {
                if (visit.count(out) == 0)
                {
                    visit[out] = visit[item] + 1;
                    out->id = i++;
                    bfs_record.push(out);
                }
                else if (visit[item] >= visit[out])
                {
                    visit[out] = visit[item] + 1;
                    if (longest_path < visit[out])
                        longest_path = visit[out];
                    bfs_record.push(out);
                }
            }
        }
        else if (longest_path < visit[item])
        {
            longest_path = visit[item];
        }
        bfs_record.pop();
    }
    init_id = i;

    // set the logic depth of all outputs
    for (auto &po:*POs) {
        visit[po] = longest_path;
    }

    // layer assignment
    this->layers.resize(longest_path);
    int layer_size = ceil(init_id / longest_path) * 2;
    for (i = 0; i < longest_path; ++i)
    {
        vector<node *> layer;
        layer.reserve(layer_size);
    }
    std::map<node *, int>::iterator iter = visit.begin();
    std::map<node *, int>::iterator iter_end = visit.end();
    for (; iter != iter_end; ++iter)
    {
        this->layers[iter->second - 1].emplace_back(iter->first);
    }
    visit.clear();
    return this->layers;
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

vector<vector<node *>> &simplify::layer_assignment(vector<node *> *PIs, vector<node *> *POs)
{
    if (PIs->empty())
    {
        std::cout << "PIs is empty in simplify.layer_assignment." << std::endl;
        return this->layers;
    }
    vector<int> visit(init_id, 0);
    vector<int> logic_depth(init_id, 0);
    this->layers.emplace_back(*PIs);
    int i = 0;
    int nums = PIs->size();
    // layer assignment, and calculate the logic depth of each node
    while (i < this->layers.size())
    {
        vector<node *> layer;
        for (int j = 0; j < this->layers[i].size(); ++j)
        {
            if (this->layers[i][j]->outs)
            {
                for (auto &out : *this->layers[i][j]->outs)
                {
                    ++visit[out->id];
                    logic_depth[out->id] = max(logic_depth[this->layers[i][j]->id] + 1, logic_depth[out->id]);
                    if (out->ins->size() == visit[out->id] && out->cell != _EXOR)
                        layer.emplace_back(out);
                }
            }
        }
        if (!layer.empty())
        {
            this->layers.emplace_back(layer);
            nums += layer.size();
        }
        ++i;
    }
    this->layers.emplace_back(*POs);
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
            ++temp_in;
        }
        if (temp_in != temp_in_end)
        {
            // son.outs.push(grandson)
            keep->outs->emplace_back(out);
        }
        else
        {
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
    for (int i = 0; i < layers->size(); ++i)
    {
        for (auto &node : (*layers->at(i)))
        {
            level[node->id] = i;
        }
    }
    int reduce = 0;
    for (int i = 0; i < layers->size() - 2; ++i)
    {
        map<Gtype, vector<node *>> record;
        for (auto &item : (*layers->at(i)))
        {
            if (item->outs && item->outs->size() > 0)
            {
                for (int j = 0; j < item->outs->size(); ++j)
                {
                    // not including output
                    if (item->outs->at(j)->cell != _EXOR)
                    {
                        if (record.count(item->outs->at(j)->cell))
                        {
                            record[item->outs->at(j)->cell].emplace_back(item->outs->at(j));
                        }
                        else
                        {
                            vector<node *> nodes;
                            nodes.emplace_back(item->outs->at(j));
                            record[item->outs->at(j)->cell] = nodes;
                            // record.insert(make_pair(item->outs->at(j)->cell, nodes));
                        }
                    }
                }
            }
        }
        for (auto &it : record)
        {
            if (it.second.size() <= 1)
            {
                continue;
            }
            // remove duplicate elements in vector
            unique_element_in_vector(it.second);
            if (it.second.size() <= 1)
            {
                continue;
            }

            if (it.first == BUF || it.first == INV)
            {
                for (int d = 1; d < it.second.size(); ++d)
                {
                    this->deduplicate(level[it.second.at(d)->id], it.second.at(0), it.second.at(d), layers);
                    ++reduce;
                }
            }
            else
            {
                for (int si = 0; si < it.second.size(); ++si)
                {
                    int candidate_size = it.second.size();
                    for (int ri = si + 1; ri < candidate_size; ++ri)
                    {
                        if (it.second.at(si)->ins->size() == it.second.at(ri)->ins->size())
                        {
                            bool flag = true;
                            for (int ii = 0; ii < it.second.at(si)->ins->size(); ++ii)
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
            it.second.clear();
        }
        record.clear();
    }
    for (int i = 0; i < layers->size(); ++i)
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