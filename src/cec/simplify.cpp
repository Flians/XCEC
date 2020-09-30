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

vector<vector<Node *>> &simplify::id_reassign_and_layered(vector<Node *> &PIs, vector<Node *> &POs)
{
    vector<vector<Node *>>().swap(this->layers);
    if (PIs.empty())
    {
        std::cout << "PIs is empty in simplify.id_reassign." << endl;
        return this->layers;
    }
    unordered_map<Node *, int> visit;
    queue<Node *> bfs_record;
    // reassign id of each node, and obtain the length of the longest path
    size_t i = 0;
    for (auto &pi : PIs)
    {
        visit[pi] = 1;
        pi->id = i++;
        bfs_record.push(pi);
    }
    int longest_path = 0;
    while (!bfs_record.empty())
    {
        Node *item = bfs_record.front();
        if (!item->outs.empty())
        {
            for (auto &out : item->outs)
            {
                if (visit.find(out) == visit.end())
                {
                    visit[out] = visit[item] + 1;
                    out->id = i++;
                    bfs_record.push(out);
                }
                else if (visit[item] >= visit[out])
                {
                    visit[out] = visit[item] + 1;
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
    std::unordered_map<Node *, int>::iterator iter = visit.begin();
    std::unordered_map<Node *, int>::iterator iter_end = visit.end();
    for (; iter != iter_end; ++iter)
    {
        this->layers[iter->second - 1].emplace_back(iter->first);
    }
    visit.clear();
    return this->layers;
}

void simplify::id_reassign()
{
    if (layers.empty())
    {
        std::cout << "PIs is empty in simplify.id_reassign." << endl;
        return;
    }
    int id = 0;
    size_t num_layer = layers.size();
    for (size_t i = 0; i < num_layer; ++i)
    {
        long last = layers[i].size() - 1;
        for (long j = 0; j <= last; ++j)
        {
            while (j <= last && !layers[i][last])
            {
                --last;
            }
            if (j > last)
                break;
            if (!layers[i][j]) {
                if(j <= last) {
                    layers[i][j] = layers[i][last];
                    layers[i][last] = nullptr;
                }
            }
            layers[i][j]->id = id++;
        }
        layers[i].resize(last + 1);
        if (layers[i].empty()) {
            layers.erase(layers.begin() + (i--));
        }
    }
    init_id = id;
}

int simplify::merge_nodes_between_networks()
{
    if (layers.empty())
    {
        cout << "The layers is empty in simplify.reduce_repeat_nodes!" << endl;
        return 0;
    }
    vector<pair<int,int>> position(init_id, {0,0});
    vector<Node*> all_node(init_id, nullptr);
    size_t num_layer = layers.size();
    for (size_t i = 0; i < num_layer; ++i)
    {
        size_t num_node = layers[i].size();
        for (size_t j = 0; j < num_node; ++j)
        {
            position[layers[i][j]->id] = {i,j};
            all_node[layers[i][j]->id] = layers[i][j];
        }
    }
    int reduce = 0;
    for (size_t i = 1; i < num_layer - 1; ++i) {
        size_t num_node = layers[i].size();
        for (size_t j = 0; j < num_node; ++j) {
            if (!layers[i][j] || layers[i][j]->ins.empty()) {
                continue;
            }
            Roaring same_id;
            bool flag = false;
            size_t num_npi = layers[i][j]->ins.size();
            for (size_t k = 0; k < num_npi; ++k) {
                Roaring tmp;
                for (auto &iout: layers[i][j]->ins[k]->outs) {
                    if (iout && iout->cell == layers[i][j]->cell && iout->ins.size() == num_npi) {
                        tmp.add(iout->id);
                    }
                }
                if (flag) {
                    same_id &= tmp;
                } else {
                    same_id = tmp;
                    flag = true;
                }
            }
            Roaring::const_iterator it = same_id.begin();
            while (it != same_id.end())
            {
                if (all_node[it.i.current_value] && it.i.current_value != layers[i][j]->id) {
                    merge_node(layers[i][j], all_node[it.i.current_value]);
                    all_node[it.i.current_value] = nullptr;
                    layers[position[it.i.current_value].first][position[it.i.current_value].second] = nullptr;
                    ++reduce;
                }
                ++it;
            }
        }
    }
    vector<Node*>().swap(all_node);
    vector<pair<int,int>>().swap(position);
    // reassign the id for all nodes
    this->id_reassign();
    std::cout << "The number of INV, BUF, and others reduction is " << reduce << std::endl;
    return reduce;
}