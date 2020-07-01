#ifndef _LIBHEAD_H_
#define _LIBHEAD_H_

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include "libstring.h"
#include <z3/z3++.h>
#include <stp/c_interface.h>


#define enumtoCharArr(val) #val

using namespace std;

extern unsigned int init_id;

// all cell types
enum Gtype
{
    _CONSTANT,
    IN,
    OUT,
    WIRE,
    _AND,
    _NAND,
    _OR,
    _NOR,
    _XOR,
    _XNOR,
    INV,
    BUF,
    _HMUX, // _HMUX \U$1 ( .O(\282 ), .I0(1'b1), .I1(\277 ), .S(\281 ));
    _DC,   // _DC \n6_5[9] ( .O(\108 ), .C(\96 ), .D(\107 ));
    _EXOR
};

enum Value
{
    L,
    H,
    X
};

extern map<string, Gtype> Value_Str;

extern map<Gtype, string> Str_Value;

extern map<Value, string> Const_Str;

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
    node() : val(X), id(init_id++), vis(0), ins(nullptr), outs(nullptr) {}
    node(string _name, Gtype _cell = WIRE, Value _val = L, int _id = (init_id++)) : name(_name), cell(_cell), val(_val), id(_id), vis(0), ins(nullptr), outs(nullptr) {}

    // destructor
    // delete this node and all edges connected to this node.
    ~node()
    {
        // cout << "~delete node: " << this->name << endl;
        if (this->ins)
        {
            for (auto in : (*this->ins))
            {
                vector<node *>::iterator temp = find(in->outs->begin(), in->outs->end(), this);
                if (temp != in->outs->end())
                    in->outs->erase(temp);
            }
            vector<node *>().swap(*this->ins);
        }
        if (this->outs)
        {
            for (auto out : (*this->outs))
            {
                vector<node *>::iterator temp = find(out->ins->begin(), out->ins->end(), this);
                if (temp != out->ins->end())
                    out->ins->erase(temp);
            }
            vector<node *>().swap(*this->outs);
        }
    }

    /* operator overload */
    // AND
    node operator&(const node &B)
    {
        node re;
        if (this->val == L || B.val == L)
        {
            re.val = L;
        }
        else
        {
            re.val = max(this->val, B.val);
        }
        // cout << this->val << " AND " << B.val << " = " << re.val << endl;
        return re;
    }

    // OR
    node operator|(const node &B)
    {
        node re;
        if (this->val == H || B.val == H)
        {
            re.val = H;
        }
        else
        {
            re.val = max(this->val, B.val);
        }
        // cout << this->val << " OR " << B.val << " = " << re.val << endl;
        return re;
    }

    // XOR
    node operator^(const node &B)
    {
        node re;
        if (this->val == H && B.val == H)
        {
            re.val = L;
        }
        else
        {
            re.val = max(this->val, B.val);
        }
        // cout << this->val << " XOR " << B.val << " = " << re.val << endl;
        return re;
    }

    // not
    node operator~()
    {
        node re;
        switch (this->val)
        {
        case L:
            re.val = H;
            break;
        case H:
            re.val = L;
            break;
        default:
            re.val = X;
        }
        // cout << this->val << " not = " << re.val << endl;
        return re;
    }

    bool operator==(const node &B)
    {
        return this->id == B.id;
    }

    bool operator<(const node &B)
    {
        if (this->outs)
        {
            if (B.outs)
            {
                return this->outs->size() < B.outs->size();
            }
            return false;
        }
        else
        {
            if (B.outs)
            {
                return true;
            }
            return this->id < B.id;
        }
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
Value HMUX(const Value &S, const Value &I0, const Value &I1);

// exor
Value EXOR(const Value &, const Value &);

Value calculate(node *g);

template <typename T>
vector<T> unique_element_in_vector(vector<T> v)
{
    typename vector<T>::iterator vector_iterator;
    sort(v.begin(), v.end());
    vector_iterator = unique(v.begin(), v.end());
    if (vector_iterator != v.end())
    {
        v.erase(vector_iterator, v.end());
    }
    return v;
}

extern Z3_context logic;
extern Z3_sort bv_sort;
extern Z3_ast z3_zero;
extern Z3_ast z3_one;
extern Z3_ast z3_x;
extern Z3_ast z3_three;
z3::params config_z3(string priority, unsigned timeout);
Z3_ast z3_mk_and(const Z3_ast &, const Z3_ast &);
Z3_ast z3_mk_and(vector<Z3_ast> &);
Z3_ast z3_mk_or(const Z3_ast &, const Z3_ast &);
Z3_ast z3_mk_or(vector<Z3_ast> &);
Z3_ast z3_mk_xor(const Z3_ast &, const Z3_ast &);
Z3_ast z3_mk_xor(vector<Z3_ast> &);
Z3_ast z3_mk_not(const Z3_ast &);
Z3_ast z3_mk_DC(const Z3_ast &C, const Z3_ast &D);
Z3_ast z3_mk_HMUX(const Z3_ast &S, const Z3_ast &I0, const Z3_ast &I1);
Z3_ast z3_mk_exor(const Z3_ast &, const Z3_ast &);

extern void cleanVP(vector<node *> *vp);
#endif