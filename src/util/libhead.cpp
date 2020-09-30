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


Node* delete_node(Node *cur) {
    if (!cur)
        return nullptr;
    if (cur->ins.empty() && cur->outs.empty() && cur->cell == WIRE) {
        // cout << cur->name << " Wire is useless in delete_node!" << endl;
        return nullptr;
    }
    if (cur->ins.size() != 1 || cur->ins.empty())
    {
        error_fout(cur->name + " Node have none or more one inputs in delete_node!");
    }
    Node *tin = cur->ins.front();
    if (!cur->outs.empty())
    {
        vector<Node *>::iterator it = cur->outs.begin();
        vector<Node *>::iterator it_end = cur->outs.end();
        while (it != it_end)
        {
            vector<Node *>::iterator temp_in = (*it)->ins.begin();
            vector<Node *>::iterator temp_in_end = (*it)->ins.end();
            while (temp_in != temp_in_end)
            {
                if (cur == (*temp_in))
                {
                    (*temp_in) = tin;
                    break;
                }
                ++temp_in;
            }
            if (temp_in != temp_in_end)
            {
                tin->outs.emplace_back(*it);
            }
            else
            {
                error_fout("There are some wrong in" + cur->name);
            }
            ++it;
        }
        vector<Node *>().swap(cur->outs);
    }
    delete cur;
    cur = nullptr;
    return tin;
}


void merge_node (Node *node, Node *repeat) {
    if (!node || !repeat) {
        cout << "There are some NULL node in libhead.merge_node!" << endl;
        return;
    }
    if (node == repeat) {
        cout << "Both nodes are the same in libhead.merge_node!" << endl;
        return;
    }
    for (auto &out : repeat->outs)
    {
        // grandson.ins.push(son)
        vector<Node *>::iterator temp_in = out->ins.begin();
        vector<Node *>::iterator temp_in_end = out->ins.end();
        while (temp_in != temp_in_end)
        {
            if (repeat == (*temp_in))
            {
                (*temp_in) = node;
                break;
            }
            ++temp_in;
        }
        if (temp_in != temp_in_end)
        {
            // son.outs.push(grandson)
            node->outs.emplace_back(out);
        }
        else
        {
            cout << "repeat can't be found in the inputs of repeat's outputs in libhead.merge_node!" << endl;
        }
    }
    vector<Node *>().swap(repeat->outs);
    delete repeat;
    repeat = nullptr;
}

Value calculate(Node *g)
{
    Value res = X;
    if (g)
    {
        res = g->ins.front()->val;
        vector<Node *>::iterator it_ = g->ins.begin();
        vector<Node *>::iterator it_end = g->ins.end() - 1;
        switch (g->cell)
        {
        case _AND:
            while (it_ != it_end)
            {
                res = res & (*(++it_))->val;
            }
            break;
        case _NAND:
            while (it_ != it_end)
            {
                res = res & (*(++it_))->val;
            }
            res = ~res;
            break;
        case _OR:
            while (it_ != it_end)
            {
                res = res | (*(++it_))->val;
            }
            break;
        case _NOR:
            while (it_ != it_end)
            {
                res = res | (*(++it_))->val;
            }
            res = ~res;
            break;
        case _XOR:
            while (it_ != it_end)
            {
                res = res ^ (*(++it_))->val;
            }
            break;
        case _XNOR:
            while (it_ != it_end)
            {
                res = res ^ (*(++it_))->val;
            }
            res = ~res;
            break;
        case INV:
            res = ~res;
            break;
        case BUF:
            break;
        case _HMUX:
            ++it_;
            res = HMUX(res, (*it_)->val, (*(it_ + 1))->val);
            break;
        case _DC:
            res = DC(res, (*(it_ + 1))->val);
            break;
        case _EXOR:
            res = EXOR(res, (*(it_ + 1))->val);
            break;
        default:
            break;
        }
    }
    else
    {
        error_fout("The node g is empty in libhead.cpp: Value calculate(node *g)");
    }
    return res;
}

void unique_element_in_vector(vector<Node *> &v)
{
    sort(v.begin(), v.end(), [](const Node *A, const Node *B) {
        if (A->id == B->id)
        {
            return A->outs.size() > B->outs.size();
        }
        else
        {
            return A->id < B->id;
        }
    });
    vector<Node *>::iterator vector_iterator = unique(v.begin(), v.end());
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
    exit(-1);
}