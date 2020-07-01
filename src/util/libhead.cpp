#include "libhead.h"

/* initial */
unsigned int init_id = 0;
map<string, Gtype> Value_Str = {
    {"constant", _CONSTANT},
    {"input", IN},
    {"output", OUT},
    {"wire", WIRE},
    {"and", _AND},
    {"nand", _NAND},
    {"or", _OR},
    {"nor", _NOR},
    {"xor", _XOR},
    {"xnor", _XNOR},
    {"not", INV},
    {"buf", BUF},
    {"_HMUX", _HMUX},
    {"_DC", _DC},
    {"_EXOR", _EXOR}};

map<Gtype, string> Str_Value = {
    {_CONSTANT, "constant"},
    {IN, "input"},
    {OUT, "output"},
    {WIRE, "wire"},
    {_AND, "and"},
    {_NAND, "nand"},
    {_OR, "or"},
    {_NOR, "nor"},
    {_XOR, "xor"},
    {_XNOR, "xnor"},
    {INV, "not"},
    {BUF, "buf"},
    {_HMUX, "_HMUX"},
    {_DC, "_DC"},
    {_EXOR, "_EXOR"}};

map<Value, string> Const_Str = {
    {L, "1'b0"},
    {H, "1'b1"},
    {X, "1'bx"}};

/* Global operator overload */
Value operator&(const Value &A, const Value &B)
{
    if (A == L || B == L)
    {
        return L;
    }
    else
    {
        return max(A, B);
    }
}

Value operator|(const Value &A, const Value &B)
{
    if (A == H || B == H)
    {
        return H;
    }
    else
    {
        return max(A, B);
    }
}

Value operator^(const Value &A, const Value &B)
{
    if (A == H && B == H)
    {
        return L;
    }
    else
    {
        return max(A, B);
    }
}

Value operator~(const Value &A)
{
    switch (A)
    {
    case L:
        return H;
    case H:
        return L;
    default:
        return X;
    }
}

inline Value DC(const Value &C, const Value &D)
{
    if (D == L)
    {
        return C;
    }
    else
    {
        return X;
    }
}

inline Value HMUX(const Value &S, const Value &I0, const Value &I1)
{
    if (S == H)
    {
        return I0 == I1 ? I0 : X;
    }
    else
    {
        return S == L ? I0 : I1;
    }
}

inline Value EXOR(const Value &A, const Value &B)
{
    if (A == X || A == B)
    {
        return L;
    }
    else
    {
        return H;
    }
}

Value calculate(node *g)
{
    node temp_g;
    if (g)
    {
        temp_g.val = g->ins->at(0)->val;
        vector<node *>::iterator it_ = g->ins->begin() + 1;
        vector<node *>::iterator it_end = g->ins->end();
        switch (g->cell)
        {
        case _AND:
            while (it_ != it_end)
            {
                temp_g = temp_g & *(*(it_++));
            }
            break;
        case _NAND:
            while (it_ != it_end)
            {
                temp_g = temp_g & *(*(it_++));
            }
            temp_g = ~temp_g;
            break;
        case _OR:
            while (it_ != it_end)
            {
                temp_g = temp_g | *(*(it_++));
            }
            break;
        case _NOR:
            while (it_ != it_end)
            {
                temp_g = temp_g | *(*(it_++));
            }
            temp_g = ~temp_g;
            break;
        case _XOR:
            while (it_ != it_end)
            {
                temp_g = temp_g ^ *(*(it_++));
            }
            break;
        case _XNOR:
            while (it_ != it_end)
            {
                temp_g = temp_g ^ *(*(it_++));
            }
            temp_g = ~temp_g;
            break;
        case INV:
            temp_g = ~temp_g;
            break;
        case BUF:
            break;
        case _HMUX:
            temp_g.val = HMUX(temp_g.val, (*it_)->val, (*(it_ + 1))->val);
            break;
        case _DC:
            temp_g.val = DC(temp_g.val, (*it_)->val);
            break;
        case _EXOR:
            temp_g.val = EXOR(temp_g.val, (*it_)->val);
            break;
        default:
            break;
        }
    }
    else
    {
        cout << "The node g is empty in libhead.cpp: Value calculate(node *g)" << endl;
        exit(-1);
    }
    return temp_g.val;
}

/**
   \brief Simpler error handler.
 */
void error_handler(Z3_context c, Z3_error_code e)
{
    printf("Error code: %d\n", e);
    fprintf(stderr, "BUG: %s.\n", "incorrect use of Z3");
    exit(1);
}

Z3_context mk_context()
{
    Z3_config cfg;
    Z3_context ctx;
    cfg = Z3_mk_config();
    Z3_set_param_value(cfg, "model", "true");

    ctx = Z3_mk_context(cfg);
    Z3_set_error_handler(ctx, error_handler);

    Z3_del_config(cfg);
    return ctx;
}

Z3_context logic = mk_context();
Z3_sort bv_sort = Z3_mk_bv_sort(logic, 2);

Z3_ast z3_zero = Z3_mk_unsigned_int(logic, 0, bv_sort);
Z3_ast z3_one = Z3_mk_unsigned_int(logic, 1, bv_sort);
Z3_ast z3_x = Z3_mk_unsigned_int(logic, 2, bv_sort);
Z3_ast z3_three = Z3_mk_unsigned_int(logic, 3, bv_sort);

Z3_ast z3_mk_and(const Z3_ast &A, const Z3_ast &B)
{
    // return z3::ite(A == z3_zero || B == z3_zero, z3_zero, z3::max(A, B));
    return Z3_mk_ite(logic, Z3_mk_eq(logic, Z3_mk_bvxor(logic, A, B), z3_three), z3_x, Z3_mk_bvand(logic, A, B));
}

Z3_ast z3_mk_and(vector<Z3_ast> &exprs)
{
    Z3_ast res = exprs[0];
    vector<Z3_ast>::iterator it_ = exprs.begin() + 1;
    vector<Z3_ast>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = z3_mk_and(res, *(it_++));
    }
    return res;
}

Z3_ast z3_mk_or(const Z3_ast &A, const Z3_ast &B)
{
    // return z3::ite(A == z3_one || B == z3_one, z3_one, z3::max(A, B));
    return Z3_mk_ite(logic, Z3_mk_eq(logic, Z3_mk_bvxor(logic, A, B), z3_three), z3_one, Z3_mk_bvor(logic, A, B));
}

Z3_ast z3_mk_or(vector<Z3_ast> &exprs)
{
    Z3_ast res = exprs[0];
    vector<Z3_ast>::iterator it_ = exprs.begin() + 1;
    vector<Z3_ast>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = z3_mk_or(res, *(it_++));
    }
    return res;
}

Z3_ast z3_mk_xor(const Z3_ast &A, const Z3_ast &B)
{
    // return z3::ite(A == z3_one && B == z3_one, z3_zero, z3::max(A, B));
    return Z3_mk_ite(logic, Z3_mk_eq(logic, Z3_mk_bvand(logic, A, B), z3_one), z3_zero, Z3_mk_ite(logic, Z3_mk_bvuge(logic, A, B), A, B));
}

Z3_ast z3_mk_xor(vector<Z3_ast> &exprs)
{
    Z3_ast res = exprs[0];
    vector<Z3_ast>::iterator it_ = exprs.begin() + 1;
    vector<Z3_ast>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = z3_mk_xor(res, *(it_++));
    }
    return res;
}

Z3_ast z3_mk_not(const Z3_ast &A)
{
    // return z3::ite(A == z3_zero, z3_one, z3::ite(A == z3_one, z3_zero, z3_x));
    // return z3::ite(A == z3_x, z3_x, A ^ z3_one);
    return Z3_mk_ite(logic, Z3_mk_eq(logic, A, z3_x), z3_x, Z3_mk_bvxor(logic, A, z3_one));
}

Z3_ast z3_mk_DC(const Z3_ast &C, const Z3_ast &D)
{
    // return z3::ite(D == z3_zero, C, z3_x);
    return Z3_mk_ite(logic, Z3_mk_eq(logic, D, z3_zero), C, z3_x);
}

Z3_ast z3_mk_HMUX(const Z3_ast &S, const Z3_ast &I0, const Z3_ast &I1)
{
    // return z3::ite(S == z3_x, z3::ite(I0 == I1, I0, z3_x), z3::ite(S == z3_zero, I0, I1));
    return Z3_mk_ite(logic, Z3_mk_eq(logic, S, z3_x),
                     Z3_mk_ite(logic, Z3_mk_eq(logic, I0, I1), I0, z3_x),
                     Z3_mk_ite(logic, Z3_mk_eq(logic, S, z3_zero), I0, I1));
}

Z3_ast z3_mk_exor(const Z3_ast &A, const Z3_ast &B)
{
    // return z3::ite(A == z3_x || A == B, z3_zero, z3_one);
    Z3_ast args[2] = {Z3_mk_eq(logic, A, z3_x), Z3_mk_eq(logic, A, B)};
    // return Z3_mk_ite(logic, Z3_mk_or(logic, 2, args), z3_zero, z3_one);
    return Z3_mk_not(logic, Z3_mk_or(logic, 2, args));
}

void cleanVP(vector<node *> *vecPtr)
{
    vector<node *>::iterator it = vecPtr->begin();
    for (; it != vecPtr->end();)
    {
        delete *it;
        it = vecPtr->erase(it);
    }
}

/**
 * Configure Z3 backend
 * 
 * @param priority: pareto, box, lex
 * @param timeout millisecond
 */
z3::params config_z3(z3::context logic, string priority, unsigned timeout)
{
    z3::params z3_param(logic);
    // http://smtlib.cs.uiowa.edu/logics-all.shtml
    logic.set("logic", "QF_BV");

    //z3_param.set(":opt.solution_prefix", "intermediate_result");
    //z3_param.set(":opt.dump_models", true);
    // z3_param.set(":opt.pb.compile_equality", true);
    // z3_param.set(":opt.priority", priority.c_str());
    z3_param.set(":timeout", timeout);
    return z3_param;
}