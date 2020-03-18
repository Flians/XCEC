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
    node temp_g;
    if (g) {
        temp_g.val =  g->ins->at(0)->val;
        vector<node *>::iterator it_ = g->ins->begin() + 1;
        vector<node *>::iterator it_end = g->ins->end();
        switch (g->cell)
        {
        case AND:
            while (it_ != it_end)
            {
                temp_g = temp_g&*(*(it_++));
            }
            break;
        case NAND:
            while (it_ != it_end)
            {
                temp_g = temp_g&*(*(it_++));
            }
            temp_g = ~temp_g;
            break;
        case OR:
            while (it_ != it_end)
            {
                temp_g = temp_g|*(*(it_++));
            }
            break;
        case NOR:
            while (it_ != it_end)
            {
                temp_g = temp_g|*(*(it_++));
            }
            temp_g = ~temp_g;
            break;
        case XOR:
            while (it_ != it_end)
            {
                temp_g = temp_g^*(*(it_++));
            }
            break;
        case NXOR:
            while (it_ != it_end)
            {
                temp_g = temp_g^*(*(it_++));
            }
            temp_g = ~temp_g;
            break;
        case INV:
            temp_g = ~temp_g;
            break;
        case BUF:
            break;
        case _MUX:
            temp_g.val = MUX(temp_g.val, (*it_)->val, (*(it_+1))->val);
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
    } else {
        cout << "The node g is empty in libhead.cpp: Value calculate(node *g)" << endl;
        exit(-1);
    }
    return temp_g.val;
}