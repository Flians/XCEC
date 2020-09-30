#include "parser.h"

parser::parser(/* args */)
{
    this->constants.reserve(3);
    for (Value val = L; val <= X; val = (Value)(val + 1))
    {
        Node *cont = new Node(Const_Str[val], _CONSTANT, val);
        this->constants.emplace_back(cont);
    }
}

parser::~parser()
{
    vector<Node *>().swap(this->PIs);
    vector<Node *>().swap(this->POs);
    vector<Node *>().swap(this->constants);
    this->wires_golden.clear();
    this->wires_revised.clear();
    this->map_PIs.clear();
    this->map_POs.clear();
    cout << "The parser is destroyed!" << endl;
}

vector<Node *> &parser::get_PIs()
{
    return this->PIs;
}

vector<Node *> &parser::get_POs()
{
    return this->POs;
}

vector<Node *> &parser::get_constants()
{
    return this->constants;
}

void parser::parse_verilog(stringstream &in, bool is_golden)
{
    string line;
    smatch match;
    regex pattern("[^ \f\n\r\t\v,;\()]+");
    size_t num_pi = this->PIs.size();
    size_t num_po = this->POs.size();
    while (getline(in, line))
    {
        line = libstring::trim(line);
        // skip annotations and empty line
        if (line.find("//") == 0 || line.empty())
            continue;
        // the wire is more than one line
        while (line.find(';') == line.npos)
        {
            string tl;
            if (!getline(in, tl))
                return;
            line += tl;
        }
        string::const_iterator iterStart = line.begin();
        string::const_iterator iterEnd = line.end();
        string item;
        if (regex_search(iterStart, iterEnd, match, pattern))
        {
            item = match[0];
            iterStart = match[0].second;
            // cout << item << endl;
            if (Value_Str.count(item))
            {
                Gtype nt = Value_Str[item];
                switch (nt)
                {
                case _MODULE:
                {
                    int io_num = count(iterStart, iterEnd, ',');
                    this->PIs.reserve(io_num);
                    this->POs.reserve(io_num);
                    break;
                }
                case IN:
                {
                    if (!is_golden)
                        continue;
                    while (regex_search(iterStart, iterEnd, match, pattern))
                    {
                        item = match[0];
                        this->map_PIs[item] = num_pi++;
                        this->PIs.emplace_back(new Node(item, IN));
                        iterStart = match[0].second;
                    }
                    break;
                }
                case OUT:
                {
                    if (!is_golden)
                        continue;
                    while (regex_search(iterStart, iterEnd, match, pattern))
                    {
                        item = match[0];
                        this->map_POs[item] = num_po++;
                        this->POs.emplace_back(new Node(item, _EXOR));
                        iterStart = match[0].second;
                    }
                    break;
                }
                case WIRE:
                {
                    while (regex_search(iterStart, iterEnd, match, pattern))
                    {
                        item = match[0];
                        if (this->map_PIs.find(item) == this->map_PIs.end() && this->map_POs.find(item) == this->map_POs.end())
                        {
                            if (is_golden)
                            {
                                this->wires_golden[item] = new Node(item, WIRE);
                            }
                            else
                            {
                                this->wires_revised[item] = new Node(item, WIRE);
                            }
                        }
                        iterStart = match[0].second;
                    }
                    break;
                }
                default:
                {
                    Node *g = new Node;
                    g->cell = nt;
                    if (regex_search(iterStart, iterEnd, match, pattern))
                    {
                        g->name = match[0];
                        iterStart = match[0].second;
                    }
                    // output port
                    if (regex_search(iterStart, iterEnd, match, pattern))
                    {
                        item = match[0];
                        iterStart = match[0].second;
                        if (item[0] == '.')
                        {
                            regex_search(iterStart, iterEnd, match, pattern);
                            item = match[0];
                            iterStart = match[0].second;
                            // cout << item << endl;
                        }
                        Node *port = nullptr;
                        if (is_golden && this->wires_golden.count(item))
                        {
                            port = this->wires_golden[item];
                        }
                        else if (!is_golden && this->wires_revised.count(item))
                        {
                            port = this->wires_revised[item];
                        }
                        else if (this->map_POs.count(item))
                        {
                            port = this->POs[this->map_POs[item]];
                        }
                        else
                        {
                            error_fout("There is no output port " + item + " in parser.parse_verilog for " + line);
                        }
                        port->ins.emplace_back(g);
                        g->outs.emplace_back(port);
                        // cout << "output port: " << port->name << endl;
                    }
                    // input port
                    while (regex_search(iterStart, iterEnd, match, pattern))
                    {
                        item = match[0];
                        iterStart = match[0].second;
                        if (item[0] == '.')
                        {
                            regex_search(iterStart, iterEnd, match, pattern);
                            item = match[0];
                            iterStart = match[0].second;
                            // cout << item << endl;
                        }
                        Node *port;
                        if (item.length() == 4 && libstring::startsWith(item, "1'b"))
                        {
                            switch (item[3])
                            {
                            case '0':
                            case '1':
                                port = this->constants[item[3] - '0'];
                                break;
                            default:
                                port = this->constants[2];
                                break;
                            }
                        }
                        else
                        {
                            if (is_golden && this->wires_golden.count(item))
                            {
                                port = this->wires_golden[item];
                            }
                            else if (!is_golden && this->wires_revised.count(item))
                            {
                                port = this->wires_revised[item];
                            }
                            else if (this->map_PIs.count(item))
                            {
                                port = this->PIs[this->map_PIs[item]];
                            }
                            else
                            {
                                error_fout("There is no input port " + item + " in parser.parse_verilog for " + line);
                            }
                        }
                        port->outs.emplace_back(g);
                        g->ins.emplace_back(port);
                        // cout << "input port: " << port->name << endl;
                    }
                    break;
                }
                }
            }
            else
            {
                error_fout("There key word '" + item + "' is unknown in parser.parse_verilog: " + line);
            }
        }
    }
}

void parser::parse(ifstream &golden, ifstream &revised)
{
    // parse the golden file
    if (!golden.is_open())
    {
        error_fout("The golden can not be open");
    }
    string buffer;
    buffer.resize(golden.seekg(0, std::ios::end).tellg());
    golden.seekg(0, std::ios::beg).read(&buffer[0], static_cast<std::streamsize>(buffer.size()));
    stringstream f_input;
    f_input.str(buffer);

    parse_verilog(f_input);
    buffer.clear();
    f_input.clear();

    // parse the revised file
    if (!revised.is_open())
    {
        error_fout("The revised can not be open");
    }

    buffer.resize(revised.seekg(0, std::ios::end).tellg());
    revised.seekg(0, std::ios::beg).read(&buffer[0], static_cast<std::streamsize>(buffer.size()));
    f_input.str(buffer);

    parse_verilog(f_input, false);
    buffer.clear();
    f_input.clear();

    // merge PIs and constants
    for (auto &con : this->constants)
    {
        if (!con->outs.empty())
        {
            this->PIs.emplace_back(con);
        }
    }

    // clear IO map
    this->map_PIs.clear();
    this->map_POs.clear();
}

void parser::parse(const string &path_golden, const string &path_revised)
{
    ifstream golden(path_golden);
    ifstream revised(path_revised);
    parse(golden, revised);
    golden.close();
    revised.close();
    cout << "The parsing process is over!" << endl;
}

void parser::clean_wires()
{
    for (auto &item : this->wires_golden)
    {
        delete_node(item.second);
    }

    for (auto &item : this->wires_revised)
    {
        delete_node(item.second);
    }
    this->wires_golden.clear();
    this->wires_revised.clear();
}

void parser::clean_buf()
{
    if (this->PIs.empty())
        return;
    stack<Node *> record;
    vector<bool> vis(init_id, 0);
    for (auto &pi : this->PIs)
    {
        record.push(pi);
        vis[pi->id] = 1;
    }
    while (!record.empty())
    {
        Node *cur = record.top();
        record.pop();
        if (!cur || cur->outs.empty())
        {
            continue;
        }
        if (cur->cell == BUF)
        {
            cur = delete_node(cur);
        }
        for (auto &out : cur->outs)
        {
            if (out && !vis[out->id])
            {
                record.push(out);
                vis[out->id] = 1;
            }
        }
    }
    vector<bool>().swap(vis);
}

void parser::printG(vector<Node *> &nodes)
{
    vector<Node *>::iterator pi = nodes.begin();
    vector<Node *>::iterator pi_end = nodes.end();
    while (pi != pi_end)
    {
        cout << (*pi)->name << " " << Str_Value[(*pi)->cell] << " " << (*pi)->val << endl;
        printG((*pi)->outs);
        ++pi;
    }
}

Node *parser::find_node_by_name(vector<Node *> &nodes, string &name)
{
    for (auto &node : nodes)
    {
        if (name == node->name)
        {
            return node;
        }
    }
    return nullptr;
}

bool parser::replace_node_by_name(vector<Node *> &nodes, Node *new_node)
{
    for (auto &node : nodes)
    {
        if (new_node->name == node->name)
        {
            node = new_node;
            return true;
        }
    }
    return false;
}
