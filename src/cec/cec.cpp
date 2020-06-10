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
    std::cout << "enumeration sort example\n";
    z3::context ctx;
    const char * enum_names[] = { "a", "b", "c" };
    z3::func_decl_vector enum_consts(ctx);
    z3::func_decl_vector enum_testers(ctx);
    z3::sort s = ctx.enumeration_sort("enumT", 3, enum_names, enum_consts, enum_testers);
    // enum_consts[0] is a func_decl of arity 0.
    // we convert it to an expression using the operator()
    z3::expr a = enum_consts[0]();
    z3::expr b = enum_consts[1]();
    z3::expr x = ctx.constant("x", s);
    z3::expr test = (x==a) && (x==b);
    std::cout << "1: " << test << std::endl;
    z3::tactic qe(ctx, "ctx-solver-simplify");
    z3::goal g(ctx);
    g.add(test);
    z3::expr res(ctx);
    z3::apply_result result_of_elimination = qe.apply(g);
    z3::goal result_goal = result_of_elimination[0];
    std::cout << "2: " << result_goal.as_expr() << std::endl;
}

void cec::evaluate_by_opensmt(vector<vector<node *> *> *layers) {
    SMTConfig c;
    CUFTheory* cuftheory = new CUFTheory(c, 2);
    THandler* thandler = new THandler(*cuftheory);
    SimpSMTSolver* solver = new SimpSMTSolver(c, *thandler);
    MainSolver* mainSolver = new MainSolver(*thandler, c, solver, "test solver");
    BVLogic& logic = cuftheory->getLogic();

    PTRef nodes[init_id];
    for (int i = 0; i < init_id; i++)
    {
        PTRef v = logic.mkBVNumVar(to_string(i).c_str());
        nodes[i] = v;
    }

    for (auto &node: (*layers->at(0))) {
        if (node->cell==_CONSTANT) {
            nodes[node->id] = logic.mkBVConst(node->val);
        }
    }

    for (int i = 1; i < layers->size(); i++)
    {
        vector<node *> *layer = layers->at(i);
        for (int j = 0; j < layer->size(); j++)
        {
            vec<PTRef> inputs;
            // ayer->at(j)->ins->at(0) is clk
            for (int k = 0; k < layer->at(j)->ins->size(); k++)
            {
                inputs.push(nodes[layer->at(j)->ins->at(k)->id]);
            }
            PTRef res;
            switch (layer->at(j)->cell)
            {
            case AND:
                res = logic.mkBVBwAnd(inputs);
                break;
            case NAND:
                break;
            case OR:
                res = logic.mkOr(inputs);
                break;
            case NOR:
                res = logic.mkOr(inputs);
                break;
            case XOR:
                res = logic.mkXor(inputs);
                break;
            case XNOR:
                res = logic.mkXor(inputs);
                break;
            case INV:
                res = logic.mkNot(inputs);
                break;
            case _HMUX:
                res = logic.mkNot(inputs);
                break;
            case _DC:
                res = logic.mkNot(inputs);
                break;
            default:
                if (inputs.size() == 0)
                {
                    cerr << "The inputs is empty! in jec.evaluate_opensmt!" << endl;
                    exit(-1);
                }
                res = inputs[0];
                break;
            }
            if (layer->at(j)->outs)
            {
                for (auto &out : (*layer->at(j)->outs))
                {
                    nodes[out->id] = res;
                }
            }
        }
    }

    vec<PTRef> outputs;
    for (auto &output : (*layers->back()))
    {
        outputs.push(nodes[output->id]);
    }
    PTRef result = logic.mkAnd(outputs);
    mainSolver->push(result);
    cout << "Running check!" << endl;
    sstat r = mainSolver->check();

    if (r == s_True)
        this->fout << "EQ" << endl;
    else if (r == s_False)
        this->fout << "NEQ" << endl;
    else if (r == s_Undef)
        this->fout << "unknown" << endl;
    else
        this->fout << "error" << endl;
}