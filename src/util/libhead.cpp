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

z3::context logic;
z3::expr z3_zero = logic.bv_val((unsigned)0, 2);
z3::expr z3_one = logic.bv_val((unsigned)1, 2);
z3::expr z3_x = logic.bv_val((unsigned)2, 2);
z3::expr z3_three = logic.bv_val((unsigned)3, 2);

z3::expr z3_mk_and(const z3::expr &A, const z3::expr &B)
{
    return z3::ite(A == z3_zero || B == z3_zero, z3_zero, z3::max(A, B));
    // return Z3_mk_ite(logic, Z3_mk_eq(logic, Z3_mk_bvxor(logic, A, B), z3_three), z3_x, A & B);
}

z3::expr z3_mk_and(vector<z3::expr> &exprs)
{
    z3::expr res = exprs[0];
    vector<z3::expr>::iterator it_ = exprs.begin() + 1;
    vector<z3::expr>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = z3_mk_and(res, *(it_++));
    }
    return res;
}

z3::expr z3_mk_or(const z3::expr &A, const z3::expr &B)
{
    // return z3::ite(A == z3_one || B == z3_one, z3_one, z3::max(A, B));
    return z3::ite(A ^ B == z3_three, z3_one, A | B);
}

z3::expr z3_mk_or(vector<z3::expr> &exprs)
{
    z3::expr res = exprs[0];
    vector<z3::expr>::iterator it_ = exprs.begin() + 1;
    vector<z3::expr>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = z3_mk_or(res, *(it_++));
    }
    return res;
}

z3::expr z3_mk_xor(const z3::expr &A, const z3::expr &B)
{
    // return z3::ite(A == z3_one && B == z3_one, z3_zero, z3::max(A, B));
    return z3::ite(A & B == z3_one, z3_zero, z3::max(A, B));
}

z3::expr z3_mk_xor(vector<z3::expr> &exprs)
{
    z3::expr res = exprs[0];
    vector<z3::expr>::iterator it_ = exprs.begin() + 1;
    vector<z3::expr>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = z3_mk_xor(res, *(it_++));
    }
    return res;
}

z3::expr z3_mk_not(const z3::expr &A)
{
    // return z3::ite(A == z3_zero, z3_one, z3::ite(A == z3_one, z3_zero, z3_x));
    return z3::ite(A == z3_x, z3_x, A^z3_one);
}

z3::expr z3_mk_DC(const z3::expr &C, const z3::expr &D)
{
    return z3::ite(D == z3_zero, C, z3_x);
}

z3::expr z3_mk_HMUX(const z3::expr &S, const z3::expr &I0, const z3::expr &I1)
{
    return z3::ite(S == z3_x, z3::ite(I0 == I1, I0, z3_x), z3::ite(S == z3_zero, I0, I1));
}

z3::expr z3_mk_exor(const z3::expr &A, const z3::expr &B)
{
    return z3::ite(A == X || A == B, z3_zero, z3_one);
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
z3::params config_z3(string priority, unsigned timeout)
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