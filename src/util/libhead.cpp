#include "libhead.h"

/* initial */
size_t init_id = 0;
FILE *fout = nullptr;
std::unordered_map<string, Gtype> Value_Str = {
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
    {"_EXOR", _EXOR},
    {"module", _MODULE}};

std::unordered_map<Gtype, string, EnumClassHash> Str_Value = {
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
    {_EXOR, "_EXOR"},
    {_MODULE, "module"}};

std::unordered_map<Value, string, EnumClassHash> Const_Str = {
    {L, "1'b0"},
    {H, "1'b1"},
    {X, "1'bx"}};

/* Global operator overload */
inline Value operator&(const Value &A, const Value &B)
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

inline Value operator|(const Value &A, const Value &B)
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

inline Value operator^(const Value &A, const Value &B)
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

inline Value operator~(const Value &A)
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

inline Value HMUX(const Value &I0, const Value &I1, const Value &S)
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

Value calculate(Node *g)
{
    Node temp_g;
    if (g)
    {
        temp_g.val = g->ins->front()->val;
        vector<Node *>::iterator it_ = g->ins->begin();
        vector<Node *>::iterator it_end = g->ins->end()-1;
        switch (g->cell)
        {
        case _AND:
            while (it_ != it_end)
            {
                temp_g = temp_g & *(*(++it_));
            }
            break;
        case _NAND:
            while (it_ != it_end)
            {
                temp_g = temp_g & *(*(++it_));
            }
            temp_g = ~temp_g;
            break;
        case _OR:
            while (it_ != it_end)
            {
                temp_g = temp_g | *(*(++it_));
            }
            break;
        case _NOR:
            while (it_ != it_end)
            {
                temp_g = temp_g | *(*(++it_));
            }
            temp_g = ~temp_g;
            break;
        case _XOR:
            while (it_ != it_end)
            {
                temp_g = temp_g ^ *(*(++it_));
            }
            break;
        case _XNOR:
            while (it_ != it_end)
            {
                temp_g = temp_g ^ *(*(++it_));
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
        cerr << "The node g is empty in libhead.cpp: Value calculate(node *g)" << endl;
        exit(-1);
    }
    return temp_g.val;
}

void unique_element_in_vector(vector<Node *> &v)
{
    sort(v.begin(), v.end(), [](const Node *A, const Node *B) {
        if (A->id == B->id)
        {
            if (A->outs)
            {
                if (B->outs)
                {
                    return A->outs->size() > B->outs->size();
                }
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return A->id < B->id;
        }
    });
    typename vector<Node *>::iterator vector_iterator = unique(v.begin(), v.end());
    if (vector_iterator != v.end())
    {
        v.erase(vector_iterator, v.end());
    }
}

void cleanVP(vector<Node *> vecPtr)
{
    if (vecPtr.empty())
        return;
    vector<Node *>::iterator it = vecPtr.begin();
    int len = vecPtr.size();
    for (int i = 0; i < len; ++i, ++it)
    {
        if (*it)
        {
            delete *it;
            *it = nullptr;
        }
    }
    vector<Node *>().swap(vecPtr);
}

void init_fout(const string &path_output)
{
    fout = fopen(path_output.c_str(), "w");
    if (NULL == fout)
    {
        cerr << "The output file can not be open!" << endl;
        exit(-1);
    }
}

int close_fout()
{
    return fclose(fout);
}

void error_fout(const string &message)
{
    cerr << "Error: " << message << endl;
    fprintf(fout, "NEQ\n%s", message.c_str());
    exit(-1);
}