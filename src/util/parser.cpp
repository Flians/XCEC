#include "parser.h"

parser::parser(/* args */)
{
    this->constants.reserve(3);
    for (Value val = L; val <= X; val = (Value)(val + 1))
    {
        node *cont = new node(Const_Str[val], _CONSTANT, val);
        this->constants.emplace_back(cont);
    }
}

parser::~parser()
{
    vector<node *>().swap(this->PIs);
    vector<node *>().swap(this->POs);
    vector<node *>().swap(this->constants);
    vector<node *>().swap(this->wires_golden);
    vector<node *>().swap(this->wires_revised);
    cout << "The parser is destroyed!" << endl;
}

vector<node *> &parser::get_PIs()
{
    return this->PIs;
}

vector<node *> &parser::get_POs()
{
    return this->POs;
}

vector<node *> &parser::get_constants()
{
    return this->constants;
}

void parser::clean_wires() {
    vector<node *>().swap(wires_golden);
    vector<node *>().swap(wires_revised);
}

node *parser::find_node_by_name(vector<node *> &nodes, string &name)
{
    for (auto node : nodes)
    {
        if (name == node->name)
        {
            return node;
        }
    }
    return nullptr;
}

bool parser::replace_node_by_name(vector<node *> &nodes, node *new_node)
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

void parser::parse_verilog(stringstream &in)
{
    string line;
    smatch match;
    regex pattern("[^ \f\n\r\t\v,;\()]+");
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
                case IN:
                {
                    while (regex_search(iterStart, iterEnd, match, pattern))
                    {
                        item = match[0];
                        this->PIs.emplace_back(new node(item, IN));
                        iterStart = match[0].second;
                    }
                    break;
                }
                case OUT:
                {
                    while (regex_search(iterStart, iterEnd, match, pattern))
                    {
                        item = match[0];
                        this->POs.emplace_back(new node(item, _EXOR));
                        iterStart = match[0].second;
                    }
                    break;
                }
                case WIRE:
                {
                    while (regex_search(iterStart, iterEnd, match, pattern))
                    {
                        item = match[0];
                        if (!find_node_by_name(this->PIs, item) && !find_node_by_name(this->POs, item))
                            this->wires_golden.emplace_back(new node(item, WIRE));
                        iterStart = match[0].second;
                    }
                    break;
                }
                default:
                {
                    node *g = new node;
                    g->ins = new vector<node *>;
                    g->outs = new vector<node *>;
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
                        node *port = find_node_by_name(this->wires_golden, item);
                        if (!port)
                        {
                            port = find_node_by_name(this->POs, item);
                            if (!port)
                            {
                                cout << "There are some troubles in parser.cpp for output port: " << line << endl;
                                exit(-1);
                            }
                        }
                        g->outs->emplace_back(port);
                        if (!port->ins)
                        {
                            port->ins = new vector<node *>[1];
                        }
                        // cout << "output port: " << port->name << endl;
                        port->ins->emplace_back(g);
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
                        node *port;
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
                            port = find_node_by_name(this->wires_golden, item);
                            if (!port)
                            {
                                port = find_node_by_name(this->PIs, item);
                                if (!port)
                                {
                                    cout << "There are some troubles in parser.cpp for input port: " << line << endl;
                                    exit(-1);
                                }
                            }
                        }
                        g->ins->emplace_back(port);
                        if (!port->outs)
                        {
                            port->outs = new vector<node *>[2];
                        }
                        // cout << "input port: " << port->name << endl;
                        port->outs->emplace_back(g);
                    }
                    break;
                }
                }
            }
            else if (item == "module")
            {
                int io_num = count(iterStart, iterEnd, ',');
                this->PIs.reserve(io_num);
                this->POs.reserve(io_num);
            }
        }
    }
}

void parser::parse_revised(stringstream &in)
{
    string line;
    smatch match;
    regex pattern("[^ \f\n\r\t\v,;\()]+");
    while (getline(in, line))
    {
        line = libstring::trim(line);
        // skip annotations and empty line
        if (line.find("//") == 0 || line.empty())
            continue;
        // the sentence is more than one line
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
                case IN:
                case OUT:
                    break;
                case WIRE:
                {
                    while (regex_search(iterStart, iterEnd, match, pattern))
                    {
                        item = match[0];
                        if (!find_node_by_name(this->PIs, item) && !find_node_by_name(this->POs, item))
                            this->wires_revised.emplace_back(new node(item, WIRE));
                        iterStart = match[0].second;
                    }
                    break;
                }
                default:
                {
                    node *g = new node;
                    g->ins = new vector<node *>;
                    g->outs = new vector<node *>;
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
                        node *port = find_node_by_name(this->wires_revised, item);
                        if (!port)
                        {
                            port = find_node_by_name(this->POs, item);
                            if (!port)
                            {
                                cout << "There are some troubles in parser.cpp for output port: " << line << endl;
                                exit(-1);
                            }
                        }
                        g->outs->emplace_back(port);
                        if (!port->ins)
                        {
                            port->ins = new vector<node *>[1];
                        }
                        // cout << "output port: " << port->name << endl;
                        port->ins->emplace_back(g);
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
                        node *port;
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
                            port = find_node_by_name(this->wires_revised, item);
                            if (!port)
                            {
                                port = find_node_by_name(this->PIs, item);
                                if (!port)
                                {
                                    cout << "There are some troubles in parser.cpp for input port: " << line << endl;
                                    exit(-1);
                                }
                            }
                        }
                        g->ins->emplace_back(port);
                        if (!port->outs)
                        {
                            port->outs = new vector<node *>[2];
                        }
                        // cout << "input port: " << port->name << endl;
                        port->outs->emplace_back(g);
                    }
                    break;
                }
                }
            }
        }
    }
}

void parser::build_miter(vector<node *> &PIs_golden, vector<node *> &POs_golden, vector<node *> &PIs_revised, vector<node *> &POs_revised)
{
    int ig_len = PIs_golden.size();
    int ir_len = PIs_revised.size();
    int og_len = POs_golden.size();
    int or_len = POs_revised.size();
    if (ig_len != ir_len || og_len != or_len)
    {
        cerr << "The golden Verilog has a different number of PIs and POs than the revised Verilog!" << endl;
        exit(-1);
    }
    vector<node *>::iterator iter = PIs_golden.begin();
    vector<node *>::iterator iter_end = PIs_golden.end();
    // merge all inputs
    while (iter != iter_end)
    {
        // cout << (*iter)->name << endl;
        node *pi = find_node_by_name(PIs_revised, (*iter)->name);
        if (!pi)
        {
            cerr << "The input pi in the golden Verilog does not exist in the revised Verilog!" << endl;
            exit(-1);
        }
        else
        {
            if (pi->outs)
            {
                vector<node *>::iterator it = pi->outs->begin();
                vector<node *>::iterator it_end = pi->outs->end();
                while (it != it_end)
                {
                    if (!replace_node_by_name(*(*it)->ins, (*iter)))
                    {
                        cerr << "There may be some wrong!" << endl;
                        exit(-1);
                    }
                    (*iter)->outs->emplace_back(*it);
                    ++it;
                }
            }
            delete pi;
            pi = nullptr;
        }
        ++iter;
    }
    vector<node *>().swap(PIs_revised);
    // merge all outputs
    iter = POs_golden.begin();
    iter_end = POs_golden.end();
    while (iter != iter_end)
    {
        node *po = find_node_by_name(POs_revised, (*iter)->name);
        if (!po)
        {
            cerr << "The output po in the golden Verilog does not exist in the revised Verilog!" << endl;
            exit(-1);
        }
        else
        {
            (*iter)->cell = _EXOR;
            for (auto &tg : *po->ins)
            {
                (*iter)->ins->emplace_back(tg);
                tg->outs->emplace_back((*iter));
            }
            delete po;
            po = nullptr;
        }
        ++iter;
    }
    vector<node *>().swap(POs_revised);
}

void parser::parse(ifstream &golden, ifstream &revised)
{
    // parse the golden file
    if (!golden.is_open())
    {
        cerr << "The golden can not be open!" << endl;
        exit(-1);
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
        cerr << "The revised can not be open!" << endl;
        exit(-1);
    }

    buffer.resize(revised.seekg(0, std::ios::end).tellg());
    revised.seekg(0, std::ios::beg).read(&buffer[0], static_cast<std::streamsize>(buffer.size()));
    f_input.str(buffer);

    parse_revised(f_input);
    buffer.clear();
    f_input.clear();

    // clear the wires
    this->clean_wires();

    // merge PIs and constants
    for (auto &con : this->constants)
    {
        if (con->outs)
        {
            this->PIs.emplace_back(con);
        }
    }
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

void parser::printG(vector<node *> *nodes)
{
    if (!nodes || nodes->size() == 0)
        return;
    vector<node *>::iterator pi = nodes->begin();
    vector<node *>::iterator pi_end = nodes->end();
    while (pi != pi_end)
    {
        cout << (*pi)->name << " " << Str_Value[(*pi)->cell] << " " << (*pi)->val << endl;
        printG((*pi)->outs);
        ++pi;
    }
}