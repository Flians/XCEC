#ifndef _BOOLECTORPROVER_H_
#define _BOOLECTORPROVER_H_

#include <iostream>
#include <assert.h>
#include <unordered_set>
#include <unordered_map>

#include "util.hpp"
#include "prover.hpp"
#include "boolector/boolector.h"

class BoolectorProver : public Prover
{
private:
    Btor *handle;
    BoolectorSort bv_sort;

public:
    BoolectorProver();
    BoolectorProver(size_t len);
    ~BoolectorProver();

    std::shared_ptr<void> prover_mk_variable(const std::string &name);

    std::shared_ptr<void> prover_mk_and(std::shared_ptr<void> const &, std::shared_ptr<void> const &);
    std::shared_ptr<void> prover_mk_or(std::shared_ptr<void> const &, std::shared_ptr<void> const &);
    std::shared_ptr<void> prover_mk_xor(std::shared_ptr<void> const &, std::shared_ptr<void> const &);
    std::shared_ptr<void> prover_mk_not(std::shared_ptr<void> const &);
    std::shared_ptr<void> prover_mk_DC(std::shared_ptr<void> const &C, std::shared_ptr<void> const &D);
    std::shared_ptr<void> prover_mk_HMUX(std::shared_ptr<void> const &I0, std::shared_ptr<void> const &I1, std::shared_ptr<void> const &S);
    std::shared_ptr<void> prover_mk_exor(std::shared_ptr<void> const &, std::shared_ptr<void> const &);
    std::shared_ptr<void> prover_mk_and_exor(std::vector<std::shared_ptr<void> > &exors);

    void handleQuery(std::shared_ptr<void> const &queryExpr, uint32_t timeout, FILE *fout);
    void handleQuery_EQ(std::shared_ptr<void> const &left, std::shared_ptr<void> const &right, uint32_t timeout, FILE *fout);
    void handleQuery_Impl(std::shared_ptr<void> const &left, std::shared_ptr<void> const &right, uint32_t timeout, FILE *fout);
    void handleQuery_Impl(std::shared_ptr<void> const &right, uint32_t timeout, FILE *fout);

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