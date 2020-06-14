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
        if (PIs->at(i)->cell == _CONSTANT)
        {
            if (!assign_PIs_value(PIs, i + 1))
                return false;
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
    }
    return true;
}

void cec::evaluate_from_PIs_to_POs(vector<node *> *PIs)
{
    if (!PIs || PIs->size() == 0)
    {
        cerr << "The vector PIs is empty!" << endl;
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
        if (g->outs)
        {
            for (auto &out : *(g->outs))
            {
                ++out->vis;
                if (out->vis == out->ins->size())
                {
                    out->vis = 0;
                    out->val = calculate(out);
                    cout << out->name << " " << out->val << endl;
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
                        cerr << out->name << " Gate have no outputs!" << endl;
                        exit(-1);
                    }
                }
            }
        }
        else
        {
            cout << "The outputs of the gate " << g->name << " are empty!" << endl;
        }
    }
    return evaluate(unique_element_in_vector(qu));
}

void cec::evaluate_from_POs_to_PIs(vector<node *> *POs)
{
}

void cec::evaluate_by_z3(vector<vector<node *> *> *layers)
{
    vector<z3::expr> nodes;
    for (int i = 0; i < init_id; i++)
    {
        z3::expr exp(logic);
        nodes.push_back(exp);
    }
    for (auto &node : (*layers->at(0)))
    {
        if (node->cell == _CONSTANT)
        {
            nodes[node->id] = logic.bv_val(node->val, 2);
        }
        else
        {
            nodes[node->id] = logic.bv_const(node->name.c_str(), 2);
        }
    }

    for (int i = 1; i < layers->size(); i++)
    {
        if (i == 62)
            cout << i << endl;
        vector<node *> *layer = layers->at(i);
        for (int j = 0; j < layer->size(); j++)
        {
            vector<z3::expr> inputs;
            for (int k = 0; k < layer->at(j)->ins->size(); k++)
            {
                inputs.push_back(nodes[layer->at(j)->ins->at(k)->id]);
            }
            z3::expr res(logic);
            switch (layer->at(j)->cell)
            {
            case AND:
                res = z3_mk_and(inputs);
                break;
            case NAND:
                res = z3_mk_not(z3_mk_and(inputs));
                break;
            case OR:
                res = z3_mk_or(inputs);
                break;
            case NOR:
                res = z3_mk_not(z3_mk_or(inputs));
                break;
            case XOR:
                res = z3_mk_xor(inputs);
                break;
            case XNOR:
                res = z3_mk_not(z3_mk_xor(inputs));
                break;
            case INV:
                res = z3_mk_not(inputs[0]);
                break;
            case _HMUX:
                res = z3_mk_HMUX(inputs[0], inputs[1], inputs[2]);
                break;
            case _DC:
                res = z3_mk_DC(inputs[0], inputs[1]);
                break;
            case _EXOR:
                res = z3_mk_exor(inputs[0], inputs[1]);
                break;
            default:
                if (inputs.size() == 0)
                {
                    cerr << "The inputs is empty! in jec.evaluate_z3!" << endl;
                    exit(-1);
                }
                res = inputs[0];
                break;
            }
            nodes[layer->at(j)->id] = res;
        }
    }

    z3::expr result = z3_zero;
    for (auto &output : (*layers->back()))
    {
        result = z3_mk_or(result, nodes[output->id]);
    }

    
    z3::solver z3_opt(logic);
    z3_opt.set(config_z3("lex", 1800000));
    z3_opt.add(result != z3_zero);
    // std::cout << z3_opt.to_smt2() << "\n";
    
/*
    z3::optimize z3_opt(logic);
    z3_opt.set(config_z3("lex", 1800000));
    z3_opt.add(result != z3_zero); 
*/
    z3::check_result sat = z3_opt.check();
    if (sat == z3::unsat)
    {
        this->fout << "EQ" << endl;
    }
    else
    {
        this->fout << "NEQ" << endl;
        z3::model m = z3_opt.get_model();
        // traversing the model
        for (unsigned i = 0; i < m.size(); i++)
        {
            z3::func_decl v = m[i];
            // assert(v.arity() == 0);
            this->fout << v.name() << " " << m.get_const_interp(v).get_numeral_int() << "\n";
        }
    }
    vector<z3::expr>().swap(nodes);
}