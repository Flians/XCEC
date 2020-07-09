#include "cec.h"

cec::cec(/* args */)
{
}

cec::cec(const string &path_output)
{
    this->fout = fopen(path_output.c_str(), "w");
    // this->fout.open(path_output, ios::out);
}

cec::~cec()
{
    fclose(this->fout);
    /*
    this->fout.flush();
    this->fout.close();
    */
}

void cec::print_PIs_value(vector<node *> *PIs, ofstream &output)
{
    for (auto pi : *PIs)
    {
        output << pi->name << " " << pi->val << endl;
    }
}
void cec::print_PIs_value(vector<node *> *PIs, FILE *output) {
    for (auto pi : *PIs)
    {
        fprintf(output, "%s %d", pi->name.c_str(), pi->val);
    }
}

bool cec::assign_PIs_value(vector<node *> *PIs, int i)
{
    if (i == PIs->size())
    {
        if (!evaluate(*PIs))
        {
            // this->fout << "NEQ" << endl;
            fprintf(this->fout, "NEQ\n");
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
        // this->fout << "EQ" << endl;
        fprintf(this->fout, "EQ\n");
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
                    if (out->outs)
                    {
                        qu.emplace_back(out);
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
    unique_element_in_vector(qu);
    return evaluate(qu);
}

void cec::evaluate_from_POs_to_PIs(vector<node *> *POs)
{
}

void cec::evaluate_by_z3(vector<vector<node *> > &layers, unsigned timeout)
{
    init_z3(timeout);
    Z3_solver z3_sol = Z3_mk_solver_for_logic(logic, Z3_mk_string_symbol(logic, "QF_BV"));
    Z3_solver_inc_ref(logic, z3_sol);

    vector<Z3_ast> nodes(init_id);
    for (auto &node : layers[0])
    {
        if (node->cell == _CONSTANT)
        {
            nodes[node->id] = Z3_mk_unsigned_int(logic, node->val, bv_sort);
        }
        else
        {
            nodes[node->id] = Z3_mk_const(logic, Z3_mk_string_symbol(logic, node->name.c_str()), bv_sort);
            Z3_solver_assert(logic, z3_sol, Z3_mk_bvule(logic, nodes[node->id], z3_one));
        }
    }

    for (int i = 1; i < layers.size(); ++i)
    {
        vector<node *> layer = layers[i];
        for (int j = 0; j < layer.size(); ++j)
        {
            vector<Z3_ast> inputs(layer[j]->ins->size());
            for (int k = 0; k < layer[j]->ins->size(); ++k)
            {
                inputs[k] = nodes[layer[j]->ins->at(k)->id];
            }
            Z3_ast res;
            switch (layer[j]->cell)
            {
            case _AND:
                res = z3_mk_and(inputs);
                break;
            case _NAND:
                res = z3_mk_not(z3_mk_and(inputs));
                break;
            case _OR:
                res = z3_mk_or(inputs);
                break;
            case _NOR:
                res = z3_mk_not(z3_mk_or(inputs));
                break;
            case _XOR:
                res = z3_mk_xor(inputs);
                break;
            case _XNOR:
                res = z3_mk_not(z3_mk_xor(inputs));
                break;
            case INV:
                res = z3_mk_not(inputs[0]);
                break;
            case _HMUX:
                res = z3_mk_HMUX(inputs[0], inputs[1], inputs[2]);
                break;
            case _DC:
                // cout << layer->at(j)->name << ", C: " << layer->at(j)->ins->front()->name << ", D: " << layer->at(j)->ins->at(1)->name << endl;
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
            nodes[layer[j]->id] = res;
        }
    }

    int i = 0;
    Z3_ast args[layers.back().size()];
    for (auto &output : layers.back())
    {
        // Z3_solver_assert(logic, z3_sol, nodes[output->id]);
        args[i++] = nodes[output->id];
    }
    vector<Z3_ast>().swap(nodes);
    Z3_ast result = Z3_mk_and(logic, layers.back().size(), args);
    Z3_solver_assert(logic, z3_sol, Z3_mk_not(logic, result));
    // printf("term: %s\n", Z3_ast_to_string(logic, result));

    check(logic, z3_sol, Z3_L_TRUE, this->fout);
    // Z3_solver_pop(logic, z3_sol, 1);
    Z3_solver_dec_ref(logic, z3_sol);
    Z3_del_context(logic);
}

void cec::evaluate_by_stp(vector<vector<node *> > &layers)
{
}