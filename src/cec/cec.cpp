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
    vector<Z3_ast> nodes(init_id);
    for (auto &node : (*layers->at(0)))
    {
        if (node->cell == _CONSTANT)
        {
            nodes[node->id] = Z3_mk_unsigned_int(logic, node->val, bv_sort);
        }
        else
        {
            nodes[node->id] = Z3_mk_const(logic, Z3_mk_string_symbol(logic, node->name.c_str()), bv_sort);
        }
    }

    for (int i = 1; i < layers->size(); i++)
    {
        vector<node *> *layer = layers->at(i);
        for (int j = 0; j < layer->size(); j++)
        {
            vector<Z3_ast> inputs(layer->at(j)->ins->size());
            for (int k = 0; k < layer->at(j)->ins->size(); k++)
            {
                inputs[k] = nodes[layer->at(j)->ins->at(k)->id];
            }
            Z3_ast res;
            switch (layer->at(j)->cell)
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
    int i = 0;
    Z3_ast args[layers->back()->size()];
    for (auto &output : (*layers->back()))
    {
        args[i++] = nodes[output->id];
    }
    Z3_ast result = Z3_mk_or(logic, layers->back()->size(), args);
    // printf("term: %s\n", Z3_ast_to_string(logic, result));

    Z3_solver z3_sol = Z3_mk_solver(logic);
    Z3_solver_inc_ref(logic, z3_sol);
    Z3_solver_assert(logic, z3_sol, result);

    Z3_model m = 0;
    switch (Z3_solver_check(logic, z3_sol))
    {
    case Z3_L_FALSE:
        this->fout << "EQ" << endl;
        break;
    case Z3_L_UNDEF:
        /* Z3 failed to prove/disprove f. */
        printf("unknown\n");
    case Z3_L_TRUE:
        /* disproved */
        this->fout << "NEQ" << endl;
        m = Z3_solver_get_model(logic, z3_sol);
        if (m)
        {
            Z3_model_inc_ref(logic, m);
            /* the model returned by Z3 is a counterexample */
            printf("counterexample:\n%s\n", Z3_model_to_string(logic, m));
            unsigned num_consts = Z3_model_get_num_consts(logic, m);
            unsigned num_funcs = Z3_model_get_num_funcs(logic, m);
            unsigned nums = num_consts + num_funcs;
            // traversing the model
            for (unsigned i = 0; i < nums; i++)
            {
                Z3_func_decl v = static_cast<unsigned>(i) < num_consts ? Z3_model_get_const_decl(logic, m, i) : Z3_model_get_func_decl(logic, m, i - num_consts);
                // assert(v.arity() == 0);
                this->fout << Z3_get_decl_name(logic, v) << " " << Z3_model_get_const_interp(logic, m, v) << "\n";
            }
        }
        break;
    }
    if (m)
        Z3_model_dec_ref(logic, m);
    vector<Z3_ast>().swap(nodes);
    Z3_solver_dec_ref(logic, z3_sol);
}

void cec::evaluate_by_stp(vector<vector<node *> *> *layers)
{
}