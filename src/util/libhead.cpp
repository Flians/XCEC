#include "libhead.h"

/* initial */
unsigned int init_id = 0;
map<string, Gtype> Value_Str = {
    {"input", IN},
    {"output", OUT},
    {"wire", WIRE},
    {"and", AND},
    {"nand", NAND},
    {"or", OR},
    {"nor", NOR},
    {"xor", XOR},
    {"nxor", NXOR},
    {"not", INV},
    {"buf", BUF},
    {"_MUX", _MUX},
    {"_DC", _DC},
    {"_EXOR", _EXOR},
    {"constant", _CONSTANT}
};

map<Gtype, string> Str_Value = {
    {IN, "input"},
    {OUT, "output"},
    {WIRE, "wire"},
    {AND, "and"},
    {NAND, "nand"},
    {OR, "or"},
    {NOR, "nor"},
    {XOR, "xor"},
    {NXOR, "nxor"},
    {INV, "not"},
    {BUF, "buf"},
    {_MUX, "_MUX"},
    {_DC, "_DC"},
    {_EXOR, "_EXOR"},
    {_CONSTANT, "constant"}
};

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

inline Value MUX(const Value &S, const Value &I0, const Value &I1)
{
    if (S==H)
    {
        return I0 == I1 ? I0 : X;
    } else
    {
        return S==L?I0:I1;
    }
}

inline Value EXOR(const Value &A, const Value &B) {
    if (A==X || A==B) {
        return L;
    } else
    {
        return H;
    }
}

Value calculate(node *g) {
    Value re = X;
    if (g) {
        switch (g->cell)
        {
        case AND:
            re = *(g->ins->at(0))&*(g->ins->at(1));
            break;
        case NAND:
            re = ~(*(g->ins->at(0))&*(g->ins->at(1)));
            break;
        case OR:
            re = *(g->ins->at(0))|*(g->ins->at(1));
            break;
        case NOR:
            re = ~(*(g->ins->at(0))|*(g->ins->at(1)));
            break;
        case XOR:
            re = *(g->ins->at(0))^*(g->ins->at(1));
            break;
        case NXOR:
            re = ~(*(g->ins->at(0))^*(g->ins->at(1)));
            break;
        case INV:
            re = ~*(g->ins->at(0));
            break;
        case BUF:
            re = g->ins->at(0)->val;
            break;
        case _MUX:
            re = MUX(g->ins->at(0)->val, g->ins->at(1)->val, g->ins->at(2)->val);
            break;
        case _DC:
            re = DC(g->ins->at(0)->val, g->ins->at(1)->val);
            break;
        case _EXOR:
            re = EXOR(g->ins->at(0)->val, g->ins->at(1)->val);
            break;
        default:
            break;
        }
    }
    return re;
}