#include "cec.h"

cec::cec(/* args */)
{
}

cec::cec(const string &path_output)
{
    this->fout.open(path_output, ios::out);
}

cec::~cec()
{
    this->fout.flush();
    this->fout.close();
}

void cec::print_PIs_value(vector<node *> *PIs, ofstream &output)
{
    for (auto pi : *PIs)
    {
        output << pi->name << " " << pi->val << endl;
    }
}

bool cec::assign_PIs_value(vector<node *> *PIs, int i)
{
    if (i == PIs->size())
    {
        if (!evaluate(*PIs))
        {
            this->fout << "NEQ" << endl;
            print_PIs_value(PIs, this->fout);
            return false;
        }
    }
    else
    {
        for (Value val = L; val < X; val = (Value)(val + 1))
        {
            PIs->at(i)->val = val;
            if (!assign_PIs_value(PIs, i + 1))
                return false;
        }
    }
    return true;
}

void cec::evaluate(vector<node *> *PIs)
{
    if (!PIs || PIs->size() == 0)
    {
        perror("The vector PIs is empty!");
        exit(-1);
    }
    if (assign_PIs_value(PIs, 0))
    {
        this->fout << "EQ" << endl;
    }
}

bool cec::evaluate(vector<node *> nodes)
{
    if (nodes.size() == 0)
        return true;
    vector<node *> qu;
    for (auto &g : nodes)
    {
        for (auto &out : *(g->outs))
        {
            ++out->vis;
            if (out->vis == out->ins->size())
            {
                out->vis = 0;
                out->val = calculate(out);
                if (out->outs)
                {
                    qu.push_back(out);
                }
                else if (out->cell == _EXOR)
                {
                    if (out->val == H)
                        return false;
                }
                else
                {
                    perror((out->name + " Gate have no outputs!").c_str());
                    exit(-1);
                }
            }
        }
    }
    return evaluate(qu);
}