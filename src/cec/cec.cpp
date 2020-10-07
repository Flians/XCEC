#include "cec.h"

cec::cec(/* args */)
{
}

cec::cec(const string &path_output)
{
    // this->fout.open(path_output, ios::out);
}

cec::~cec()
{
    /*
    this->fout.flush();
    this->fout.close();
    */
}

void cec::print_PIs_value(vector<Node *> *PIs, ofstream &output)
{
    for (auto &pi : *PIs)
    {
        output << pi->name << " " << pi->val << endl;
    }
}
void cec::print_PIs_value(vector<Node *> *PIs, FILE *output)
{
    for (auto &pi : *PIs)
    {
        fprintf(output, "%s %d", pi->name.c_str(), pi->val);
    }
}

bool cec::assign_PIs_value(vector<Node *> *PIs, size_t i)
{
    if (i == PIs->size())
    {
        if (!evaluate(*PIs))
        {
            // this->fout << "NEQ" << endl;
            fprintf(fout, "NEQ\n");
            print_PIs_value(PIs, fout);
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

void cec::evaluate_from_PIs_to_POs(vector<Node *> *PIs)
{
    if (!PIs || PIs->size() == 0)
    {
        cerr << "The vector PIs is empty!" << endl;
        exit(-1);
    }
    if (assign_PIs_value(PIs, 0))
    {
        // this->fout << "EQ" << endl;
        fprintf(fout, "EQ\n");
    }
}

bool cec::evaluate(vector<Node *> nodes)
{
    if (nodes.size() == 0)
        return true;
    vector<Node *> qu;
    for (auto &g : nodes)
    {
        for (auto &out : g->outs)
        {
            ++out->vis;
            if (out->vis == out->ins.size())
            {
                out->vis = 0;
                out->val = calculate(out);
                if (!out->outs.empty())
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
    unique_element_in_vector(qu);
    return evaluate(qu);
}

void cec::evaluate_from_POs_to_PIs(vector<Node *> *POs)
{
}

void cec::evaluate_by_z3(vector<vector<Node *>> &layers, int timeout, int max_conflicts)
{
    Z3Prover z3_prover(timeout);
    // z3_prover.test();
    vector<Z3_ast> nodes(init_id);
    for (auto &node : layers[0])
    {
        if (node->cell == _CONSTANT)
        {
            switch (node->val)
            {
            case L:
                nodes[node->id] = z3_prover.z3_zero;
                break;
            case H:
                nodes[node->id] = z3_prover.z3_one;
                break;
            default:
                nodes[node->id] = z3_prover.z3_x;
                break;
            }
        }
        else
        {
            nodes[node->id] = z3_prover.z3_mk_variable(node->name);
        }
    }

    for (size_t i = 1; i < layers.size(); ++i)
    {
        vector<Node *> layer = layers[i];
        for (size_t j = 0; j < layer.size(); ++j)
        {
            vector<Z3_ast> inputs(layer[j]->ins.size());
            for (size_t k = 0; k < layer[j]->ins.size(); ++k)
            {
                inputs[k] = nodes[layer[j]->ins[k]->id];
            }
            Z3_ast res;
            switch (layer[j]->cell)
            {
            case _AND:
                res = z3_prover.z3_mk_and(inputs);
                break;
            case _NAND:
                res = z3_prover.z3_mk_not(z3_prover.z3_mk_and(inputs));
                break;
            case _OR:
                res = z3_prover.z3_mk_or(inputs);
                break;
            case _NOR:
                res = z3_prover.z3_mk_not(z3_prover.z3_mk_or(inputs));
                break;
            case _XOR:
                res = z3_prover.z3_mk_xor(inputs);
                break;
            case _XNOR:
                res = z3_prover.z3_mk_not(z3_prover.z3_mk_xor(inputs));
                break;
            case INV:
                res = z3_prover.z3_mk_not(inputs[0]);
                break;
            case _HMUX:
                res = z3_prover.z3_mk_HMUX(inputs[0], inputs[1], inputs[2]);
                break;
            case _DC:
                res = z3_prover.z3_mk_DC(inputs[0], inputs[1]);
                break;
            case _EXOR:
                res = z3_prover.z3_mk_exor(inputs[0], inputs[1]);
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
    Z3_ast result = Z3_mk_and(z3_prover.logic, layers.back().size(), args);
    // printf("term: %s\n", Z3_ast_to_string(logic, result));
    z3_prover.check(result, fout);
}

void cec::evaluate_by_stp(vector<vector<Node *>> &layers, int timeout, int max_conflicts)
{
    STPProver stp_prover;
    // stp_prover.test();
    stp_prover.init_exprs(layers[0].size());
    vector<Expr> nodes(init_id);
    for (auto &node : layers[0])
    {
        if (node->cell == _CONSTANT)
        {
            switch (node->val)
            {
            case L:
                nodes[node->id] = stp_prover.stp_zero;
                break;
            case H:
                nodes[node->id] = stp_prover.stp_one;
                break;
            default:
                nodes[node->id] = stp_prover.stp_x;
                break;
            }
        }
        else
        {
            nodes[node->id] = stp_prover.stp_mk_variable(node->name);
        }
    }

    for (size_t i = 1; i < layers.size(); ++i)
    {
        vector<Node *> layer = layers[i];
        for (size_t j = 0; j < layer.size(); ++j)
        {
            vector<Expr> inputs(layer[j]->ins.size());
            for (size_t k = 0; k < layer[j]->ins.size(); ++k)
            {
                inputs[k] = nodes[layer[j]->ins[k]->id];
            }
            Expr res;
            switch (layer[j]->cell)
            {
            case _AND:
                res = stp_prover.stp_mk_and(inputs);
                break;
            case _NAND:
                res = stp_prover.stp_mk_not(stp_prover.stp_mk_and(inputs));
                break;
            case _OR:
                res = stp_prover.stp_mk_or(inputs);
                break;
            case _NOR:
                res = stp_prover.stp_mk_not(stp_prover.stp_mk_or(inputs));
                break;
            case _XOR:
                res = stp_prover.stp_mk_xor(inputs);
                break;
            case _XNOR:
                res = stp_prover.stp_mk_not(stp_prover.stp_mk_xor(inputs));
                break;
            case INV:
                res = stp_prover.stp_mk_not(inputs[0]);
                break;
            case _HMUX:
                res = stp_prover.stp_mk_HMUX(inputs[0], inputs[1], inputs[2]);
                break;
            case _DC:
                // cout << layer[j]->name << ", C: " << layer[j]->ins->front()->name << ", D: " << layer[j]->ins->at(1)->name << endl;
                res = stp_prover.stp_mk_DC(inputs[0], inputs[1]);
                break;
            case _EXOR:
                res = stp_prover.stp_mk_exor(inputs[0], inputs[1]);
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
    Expr args[layers.back().size()];
    for (auto &output : layers.back())
    {
        args[i++] = nodes[output->id];
    }
    stp_prover.handleQuery_Impl(stp_prover.stp_mk_and_exor(args, layers.back().size()), timeout, max_conflicts, fout);
    vector<Expr>().swap(nodes);
}

void cec::evaluate_by_boolector(vector<vector<Node *>> &layers, int timeout, int max_conflicts)
{
    Prover *ble_prover = new BoolectorProver();

    vector<void *> nodes(init_id);
    for (auto &node : layers[0])
    {
        if (node->cell == _CONSTANT)
        {
            switch (node->val)
            {
            case L:
                nodes[node->id] = ble_prover->prover_zero;
                break;
            case H:
                nodes[node->id] = ble_prover->prover_one;
                break;
            default:
                nodes[node->id] = ble_prover->prover_x;
                break;
            }
        }
        else
        {
            nodes[node->id] = ble_prover->prover_mk_variable(node->name);
        }
    }

    for (size_t i = 1; i < layers.size(); ++i)
    {
        vector<Node *> layer = layers[i];
        for (size_t j = 0; j < layer.size(); ++j)
        {
            vector<void *> inputs(layer[j]->ins.size());
            for (size_t k = 0; k < layer[j]->ins.size(); ++k)
            {
                inputs[k] = nodes[layer[j]->ins[k]->id];
            }
            void *res;
            switch (layer[j]->cell)
            {
            case _AND:
                res = ble_prover->prover_mk_and(inputs);
                break;
            case _NAND:
                res = ble_prover->prover_mk_not(ble_prover->prover_mk_and(inputs));
                break;
            case _OR:
                res = ble_prover->prover_mk_or(inputs);
                break;
            case _NOR:
                res = ble_prover->prover_mk_not(ble_prover->prover_mk_or(inputs));
                break;
            case _XOR:
                res = ble_prover->prover_mk_xor(inputs);
                break;
            case _XNOR:
                res = ble_prover->prover_mk_not(ble_prover->prover_mk_xor(inputs));
                break;
            case INV:
                res = ble_prover->prover_mk_not(inputs[0]);
                break;
            case _HMUX:
                res = ble_prover->prover_mk_HMUX(inputs[0], inputs[1], inputs[2]);
                break;
            case _DC:
                // cout << layer[j]->name << ", C: " << layer[j]->ins->front()->name << ", D: " << layer[j]->ins->at(1)->name << endl;
                res = ble_prover->prover_mk_DC(inputs[0], inputs[1]);
                break;
            case _EXOR:
                res = ble_prover->prover_mk_exor(inputs[0], inputs[1]);
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
            vector<void *>().swap(inputs);
        }
    }

    
    int i = 0;
    std::vector<void *> args(layers.back().size(), NULL);
    for (auto &output : layers.back())
    {
        args[i++] = nodes[output->id];
    }
    ble_prover->handleQuery_Impl(ble_prover->prover_mk_and_exor(args), timeout, max_conflicts, fout);
    // ble_prover->handleQuery_incremental(args, timeout, max_conflicts, fout);
    vector<void *>().swap(nodes);
    vector<void *>().swap(args);
    delete ble_prover;
}