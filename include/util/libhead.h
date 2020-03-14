#ifndef _LIBHEAD_H_
#define _LIBHEAD_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <regex>
#include <algorithm>

#define enumtoCharArr(val) #val

using namespace std;

extern unsigned int init_id;

// all cell types
enum Gtype
{
    IN,
    OUT,
    WIRE,
    AND,
    NAND,
    OR,
    NOR,
    XOR,
    NXOR,
    INV,
    BUF,
    _MUX,
    _DC,
    _EXOR,
    _CONSTANT,
};

extern map<string, Gtype> Value_Str;

extern map<Gtype, string> Str_Value;

enum Value
{
    L,
    H,
    X
};

struct node
{
    // the name of the gate
    string name;
    // the type of the gate
    Gtype cell;
    Value val;
    int id;
    // record the number of times the node is visited
    int vis;
    vector<node *> *ins;
    vector<node *> *outs;
    
    // constructor
    node() : val(L), id(init_id++), vis(0), ins(NULL), outs(NULL) {}
    node(string _name, Gtype _cell = WIRE, Value _val = L, int _id = (init_id++)) : name(_name), cell(_cell), val(_val), id(_id), vis(0), ins(NULL), outs(NULL) {}

    // destructor
    // delete this node and all edges connected to this node.
    ~node()
    {
        cout << "~delete node: " << this->name << endl;
        if (this->ins)
        {
            for (auto in : (*ins))
            {
                vector<node *>::iterator temp = find(in->outs->begin(), in->outs->end(), this);
                if (temp != in->outs->end())
                    in->outs->erase(temp);
            }
            vector<node *>().swap(*ins);
        }
        if (this->outs)
        {
            for (auto out : (*outs))
            {
                vector<node *>::iterator temp = find(out->ins->begin(), out->ins->end(), this);
                if (temp != out->ins->end())
                    out->ins->erase(temp);
            }
            vector<node *>().swap(*outs);
        }
    }

    /* operator overload */
    // AND
    const Value operator&(const node &B)
    {
        if (this->val == L || B.val == L)
        {
            return L;
        }
        else
        {
            return max(this->val, B.val);
        }
    }

    // OR
    Value operator|(const node &B)
    {
        if (this->val == H || B.val == H)
        {
            return H;
        }
        else
        {
            return max(this->val, B.val);
        }
    }

    // XOR
    Value operator^(const node &B)
    {
        if (this->val == H && B.val == H)
        {
            return L;
        }
        else
        {
            return max(this->val, B.val);
        }
    }

    // not
    Value operator~()
    {
        switch (this->val)
        {
        case L:
            return H;
        case H:
            return L;
        default:
            return X;
        }
    }

    bool operator==(const node &B)
    {
        return this->name == B.name;
    }
};

/* Global operator overload */
// and
Value operator&(const Value &, const Value &);

// or
Value operator|(const Value &, const Value &);

// xor
Value operator^(const Value &, const Value &);

// not
Value operator~(const Value &);

/* O=D?1'bx:C */
Value DC(const Value &C, const Value &D);

/* O=S?I1:I0 */
Value MUX(const Value &S, const Value &I0, const Value &I1);

// exor
Value EXOR(const Value &, const Value &);

Value calculate(node *g);

#endif