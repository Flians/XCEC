#include "simplify.h"

simplify::simplify(/* args */)
{
}

simplify::~simplify()
{
    /* free up space */
    for (auto &item : this->layers)
    {
        cleanVP(item);
    }
    vector<vector<Node *>>().swap(this->layers);
}

vector<vector<Node *>> &simplify::get_layers()
{
    return this->layers;
}

bool simplify::replace_node_by_id(vector<Node *> *nodes, Node *new_node, int id)
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
void simplify::clean_wire_buf(vector<Node *> *PIs)
{
    if (!PIs || PIs->empty())
        return;
    int len = PIs->size();
    for (int i = 0; i < len; ++i)
    {
        Node *pi = PIs->at(i);
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
            Node *tin = pi->ins->front();
            vector<Node *>::iterator it = pi->outs->begin();
            vector<Node *>::iterator it_end = pi->outs->end();
            while (it != it_end)
            {
                vector<Node *>::iterator temp_in = (*it)->ins->begin();
                vector<Node *>::iterator temp_in_end = (*it)->ins->end();
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
            vector<Node *>().swap(*(pi->outs));
            delete pi;
            pi = nullptr;
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

vector<vector<Node *>> &simplify::id_reassign_and_layered(vector<Node *> &PIs, vector<Node *> &POs)
{
    if (PIs.empty())
    {
        std::cout << "PIs is empty in simplify.id_reassign." << endl;
        return this->layers;
    }
    map<Node *, int> visit;
    queue<Node *> bfs_record;
    // reassign id of each node, and obtain the length of the longest path
    int i = 0;
    for (auto pi : PIs)
    {
        visit[pi] = 1;
        pi->id = i++;
        bfs_record.push(pi);
    }
    int longest_path = 0;
    while (!bfs_record.empty())
    {
        Node *item = bfs_record.front();
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
    for (auto &po : POs)
    {
        visit[po] = longest_path;
    }

    // layer assignment
    this->layers.resize(longest_path);
    int layer_size = ceil(init_id / longest_path) * 2;
    for (i = 0; i < longest_path; ++i)
    {
        vector<Node *> layer;
        layer.reserve(layer_size);
    }
    std::map<Node *, int>::iterator iter = visit.begin();
    std::map<Node *, int>::iterator iter_end = visit.end();
    for (; iter != iter_end; ++iter)
    {
        this->layers[iter->second - 1].emplace_back(iter->first);
    }
    visit.clear();
    return this->layers;
}

void simplify::id_reassign(vector<vector<Node *>> &layers)
{
    if (layers.empty())
    {
        std::cout << "PIs is empty in simplify.id_reassign." << endl;
        return;
    }
    int id = 0;
    for (int i = 0; i < layers.size(); ++i)
    {
        for (int j = 0; j < layers[i].size(); ++j)
        {
            layers[i][j]->id = id++;
        }
    }
    init_id = id;
}

void simplify::id_reassign(vector<Node *> &PIs)
{
    if (PIs.empty())
    {
        std::cout << "PIs is empty in simplify.id_reassign." << endl;
        return;
    }
    unordered_map<Node *, bool> visit;
    queue<Node *> bfs_record;
    int i = 0;
    for (auto pi : PIs)
    {
        visit[pi] = true;
        pi->id = i++;
        bfs_record.push(pi);
    }
    while (!bfs_record.empty())
    {
        Node *item = bfs_record.front();
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

vector<vector<Node *>> &simplify::layer_assignment(vector<Node *> &PIs, vector<Node *> &POs)
{
    if (PIs.empty())
    {
        std::cout << "PIs is empty in simplify.layer_assignment." << std::endl;
        return this->layers;
    }
    vector<int> visit(init_id, 0);
    vector<int> logic_depth(init_id, 0);
    this->layers.emplace_back(PIs);
    int i = 0;
    int nums = PIs.size();
    // layer assignment, and calculate the logic depth of each node
    while (i < this->layers.size())
    {
        vector<Node *> layer;
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
    this->layers.emplace_back(POs);
    nums += POs.size();
    vector<int>().swap(visit);
    vector<int>().swap(logic_depth);
    std::cout << "The layer assignment is over!" << std::endl;
    return layers;
}

void simplify::deduplicate(int i, Node *keep, Node *dupl, vector<vector<Node *>> &layers, vector<Roaring> &nbrs)
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
        vector<Node *>::iterator temp_in = out->ins->begin();
        vector<Node *>::iterator temp_in_end = out->ins->end();
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
            nbrs[out->id].remove(dupl->id);
            nbrs[out->id].add(keep->id);
        }
        else
        {
            cout << "dupl is not equal with keep in simplify.deduplicate." << endl;
        }
    }
    vector<Node *>::iterator it = find(layers[i].begin(), layers[i].end(), dupl);
    // layers->at(i)->erase(it);
    *it = *(layers[i].end() - 1);
    *(layers[i].end() - 1) = nullptr;
    layers[i].resize(layers[i].size() - 1);
    vector<Node *>().swap(*(dupl->outs));
    nbrs[dupl->id].~Roaring();
    delete dupl;
    dupl = nullptr;
}

void simplify::reduce_repeat_nodes(vector<vector<Node *>> &layers)
{
    if (layers.empty())
    {
        cout << "The layers is empty in simplify.reduce_repeat_nodes!" << endl;
        return;
    }
    vector<int> level(init_id, 0);
    vector<Roaring> nbrs(init_id);
    for (int i = 0; i < layers.size(); ++i)
    {
        for (auto &node_ : layers[i])
        {
            level[node_->id] = i;
            if (node_->ins)
            {
                for (auto &in : *node_->ins)
                {
                    nbrs[node_->id].add(in->id);
                }
            }
        }
    }
    int reduce = 0;
    for (int i = 0; i < layers.size() - 2; ++i)
    {
        vector<vector<Node *>> record(15);
        for (auto &item : layers[i])
        {
            if (item->outs && item->outs->size() > 0)
            {
                for (int j = 0; j < item->outs->size(); ++j)
                {
                    // not including output
                    if (item->outs->at(j)->cell != _EXOR)
                    {
                        record[item->outs->at(j)->cell].emplace_back(item->outs->at(j));
                    }
                }
            }
        }
        for (auto &item : record)
        {
            if (item.size() <= 1)
            {
                continue;
            }
            // remove duplicate elements in vector
            unique_element_in_vector(item);
            if (item.size() <= 1)
            {
                continue;
            }

            if (item[0]->cell == BUF || item[0]->cell == INV)
            {
                for (int d = 1; d < item.size(); ++d)
                {
                    this->deduplicate(level[item[d]->id], item.front(), item[d], layers, nbrs);
                    ++reduce;
                }
            }
            else
            {
                for (int si = 0; si < item.size(); ++si)
                {
                    int candidate_size = item.size();
                    for (int ri = si + 1; ri < candidate_size; ++ri)
                    {
                        if (nbrs[item[si]->id] == nbrs[item[ri]->id])
                        {
                            this->deduplicate(level[item[ri]->id], item[si], item[ri], layers, nbrs);
                            // item.erase(it.begin() + ri);
                            --candidate_size;
                            *(item.begin() + ri) = *(item.begin() + candidate_size);
                            *(item.begin() + candidate_size) = nullptr;
                            --ri;
                        }
                    }
                    if (item.size() > candidate_size)
                    {
                        reduce += item.size() - candidate_size;
                        item.resize(candidate_size);
                    }
                }
            }
            item.clear();
        }
        record.clear();
    }
    for (int i = 0; i < layers.size(); ++i)
    {
        if (layers[i].empty())
        {
            layers.erase(layers.begin() + i);
            --i;
        }
    }
    vector<int>().swap(level);
    vector<Roaring>().swap(nbrs);

    std::cout << "The number of INV, BUF, and others reduction is " << reduce << std::endl;
}