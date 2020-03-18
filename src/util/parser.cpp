#include "parser.h"

node *parser::find_node_by_name(vector<node *> *nodes, string name)
{
    for (auto node : *nodes)
    {
        if (name == node->name)
        {
            return node;
        }
    }
    return NULL;
}

bool parser::replace_node_by_name(vector<node *> *nodes, node *new_node)
{
    for (auto &node : *nodes)
    {
        if (new_node->name == node->name)
        {
            node = new_node;
            return true;
        }
    }
    return false;
}

void parser::parse_verilog(ifstream &in, vector<node *> *PIs, vector<node *> *POs, vector<node *> *wires, vector<node *> *gates)
{
    string line;
    smatch match;
    regex pattern("\\w+");
    while (getline(in, line))
    {
        // the wire is more than one line
        while (line.find(';')==line.npos) {
            string tl;
            if(!getline(in, tl))
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
                        PIs->push_back(new node(item, IN));
                        iterStart = match[0].second;
                    }
                    break;
                }
                case OUT:
                {
                    while (regex_search(iterStart, iterEnd, match, pattern))
                    {
                        item = match[0];
                        POs->push_back(new node(item, OUT));
                        iterStart = match[0].second;
                    }
                    break;
                }
                case WIRE:
                {
                    while (regex_search(iterStart, iterEnd, match, pattern))
                    {
                        item = match[0];
                        if (!find_node_by_name(PIs, item) && !find_node_by_name(POs, item))
                            wires->push_back(new node(item, WIRE));
                        iterStart = match[0].second;
                    }
                    break;
                }
                default:
                {
                    node *g = new node;
                    g->ins = new vector<node *>[2];
                    g->outs = new vector<node *>[1];
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
                        node *port = find_node_by_name(wires, item);
                        if (port)
                        {
                            g->outs->push_back(port);
                        }
                        else
                        {
                            port = find_node_by_name(POs, item);
                            if (port)
                            {
                                g->outs->push_back(port);
                            }
                            else
                            {
                                cout << "There are some troubles in parser.cpp for output port: " << line << endl;
                                exit(-1);
                            }
                        }
                        if (!port->ins)
                        {
                            port->ins = new vector<node *>[1];
                        }
                        // cout << "output port: " << port->name << endl;
                        port->ins->push_back(g);
                    }
                    // input port
                    while (regex_search(iterStart, iterEnd, match, pattern))
                    {
                        item = match[0];
                        iterStart = match[0].second;
                        node *port = find_node_by_name(wires, item);
                        if (port)
                        {
                            g->ins->push_back(port);
                        }
                        else
                        {
                            port = find_node_by_name(PIs, item);
                            if (port)
                            {
                                g->ins->push_back(port);
                            }
                            else
                            {
                                cout << "There are some troubles in parser.cpp for input port: " << line << endl;
                                exit(-1);
                            }
                        }
                        if (!port->outs)
                        {
                            port->outs = new vector<node *>[2];
                        }
                        // cout << "input port: " << port->name << endl;
                        port->outs->push_back(g);
                    }
                    gates->push_back(g);
                    break;
                }
                }
            }
        }
    }
}

vector<node *> *parser::build_miter(vector<node *> *PIs_golden, vector<node *> *POs_golden, vector<node *> *PIs_revised, vector<node *> *POs_revised)
{
    int ig_len = PIs_golden->size();
    int ir_len = PIs_revised->size();
    int og_len = POs_golden->size();
    int or_len = POs_revised->size();
    if (ig_len != ir_len || og_len != or_len)
    {
        perror("The golden Verilog has a different number of PIs and POs than the revised Verilog!");
        exit(-1);
    }
    // POs
    vector<node *> *POs = new vector<node *>[og_len];
    vector<node *>::iterator iter = PIs_golden->begin();
    // merge all inputs
    while (iter != PIs_golden->end())
    {
        node *pi = find_node_by_name(PIs_revised, (*iter)->name);
        if (!pi)
        {
            perror("The input pi in the golden Verilog does not exist in the revised Verilog!");
            exit(-1);
        }
        else
        {
            vector<node *>::iterator it = pi->outs->begin();
            while (it != pi->outs->end())
            {
                if (!replace_node_by_name((*it)->ins, (*iter)))
                {
                    perror("There may be some wrong!");
                    exit(-1);
                }
                (*iter)->outs->push_back(*it);
                it++;
            }
            delete pi;
        }
        iter++;
    }
    vector<node *>().swap(*PIs_revised);
    // merge all outputs
    iter = POs_golden->begin();
    int i = 0;
    while (iter != POs_golden->end())
    {
        node *po = find_node_by_name(POs_revised, (*iter)->name);
        if (!po)
        {
            perror("The output po in the golden Verilog does not exist in the revised Verilog!");
            exit(-1);
        }
        else
        {
            node *exor_ = new node("exor_" + to_string(i++), _EXOR);
            exor_->ins = new vector<node *>[2];
            exor_->ins->push_back(*iter);
            exor_->ins->push_back(po);
            (*iter)->outs = new vector<node *>[1];
            (*iter)->outs->push_back(exor_);
            // change the type of this node from OUT into WIRE
            (*iter)->cell = WIRE;
            po->outs = new vector<node *>[1];
            po->outs->push_back(exor_);
            po->cell = WIRE;
            POs->push_back(exor_);
        }
        iter++;
    }
    vector<node *>().swap(*POs_golden);
    vector<node *>().swap(*POs_revised);
    return POs;
}

void parser::parse(ifstream &golden, ifstream &revised, vector<node *> *&PIs, vector<node *> *&POs)
{
    if (!golden.is_open())
    {
        perror("The golden can not be open!");
        exit(-1);
    }
    if (!PIs)
        PIs = new vector<node *>[32];
    vector<node *> *POs_golden = new vector<node *>[32];
    vector<node *> *wires_golden = new vector<node *>[32];
    vector<node *> *gates_golden = new vector<node *>[64];
    parse_verilog(golden, PIs, POs_golden, wires_golden, gates_golden);

    if (!revised.is_open())
    {
        perror("The revised can not be open!");
        exit(-1);
    }
    vector<node *> *PIs_revised = new vector<node *>[32];
    vector<node *> *POs_revised = new vector<node *>[32];
    vector<node *> *wires_revised = new vector<node *>[32];
    vector<node *> *gates_revised = new vector<node *>[64];
    parse_verilog(revised, PIs_revised, POs_revised, wires_revised, gates_revised);

    POs = build_miter(PIs, POs_golden, PIs_revised, POs_revised);
}

void parser::parse(const string &path_golden, const string &path_revised, vector<node *> *&PIs, vector<node *> *&POs)
{
    ifstream golden(path_golden);
    ifstream revised(path_revised);
    parse(golden, revised, PIs, POs);
    golden.close();
    revised.close();
}

void parser::printG(vector<node *> *nodes)
{
    if (!nodes || nodes->size() == 0)
        return;
    int len = nodes->size();
    vector<node *>::iterator pi = nodes->begin();
    while (pi != nodes->end())
    {
        cout << (*pi)->name << " " << Str_Value[(*pi)->cell] << " " << (*pi)->val << endl;
        printG((*pi)->outs);
        pi++;
    }
}