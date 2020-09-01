#ifndef _BOOLECTORPROVER_H_
#define _BOOLECTORPROVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <assert.h>
#include <unordered_set>
#include <unordered_map>

#include "boolector/boolector.h"

class BoolectorProver
{
private:
    Btor *btor;
    BoolectorSort bsort;
    std::unordered_set<BoolectorNode *> exprs;
    std::vector<BoolectorNode *> assert_exprs;
    std::vector<BoolectorNode *> in_exprs;

public:
    BoolectorNode *ble_zero;
    BoolectorNode *ble_one;
    BoolectorNode *ble_x;
    BoolectorNode *ble_undefined;

    BoolectorProver(/* args */);
    ~BoolectorProver();

    BoolectorNode *boolector_mk_variable(std::string &name);

    BoolectorNode *boolector_mk_and(BoolectorNode *, BoolectorNode *);
    BoolectorNode *boolector_mk_and(std::vector<BoolectorNode *> &);
    BoolectorNode *boolector_mk_or(BoolectorNode *, BoolectorNode *);
    BoolectorNode *boolector_mk_or(std::vector<BoolectorNode *> &);
    BoolectorNode *boolector_mk_xor(BoolectorNode *, BoolectorNode *);
    BoolectorNode *boolector_mk_xor(std::vector<BoolectorNode *> &);
    BoolectorNode *boolector_mk_not(BoolectorNode *);
    BoolectorNode *boolector_mk_DC(BoolectorNode *C, BoolectorNode *D);
    BoolectorNode *boolector_mk_HMUX(BoolectorNode *I0, BoolectorNode *I1, BoolectorNode *S);
    BoolectorNode *boolector_mk_exor(BoolectorNode *, BoolectorNode *);
    BoolectorNode *boolector_mk_and_exor(std::vector<BoolectorNode *> &);

    void handleQuery(BoolectorNode *queryExpr, uint32_t timeout, FILE *fout);
    void handleQuery_EQ(BoolectorNode *left, BoolectorNode *right, uint32_t timeout, FILE *fout);
    void handleQuery_Impl(BoolectorNode *left, BoolectorNode *right, uint32_t timeout, FILE *fout);
    void handleQuery_Impl(BoolectorNode *right, uint32_t timeout, FILE *fout);

    /***************** test every operators **********************/
    void test();
    void test_AND();
    void test_NAND();
    void test_OR();
    void test_NOR();
    void test_XOR();
    void test_XNOR();
    void test_INV();
    void test_DC();
    void test_HMUX();
    void test_EXOR();
};

#endif