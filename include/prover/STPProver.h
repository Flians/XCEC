#ifndef _STPPROVER_H_
#define _STPPROVER_H_

#include "prover.hpp"
#include "stp/c_interface.h"

class STPProver
{
private:
    VC handle;
    Type bvsort;

public:
    STPProver();
    STPProver(size_t len);
    ~STPProver();

    // Error handler
    static void errorHandler(const char *err_msg);

    void *prover_mk_variable(const std::string &name);

    void *prover_mk_and(void *const &, void *const &);
    void *prover_mk_or(void *const &, void *const &);
    void *prover_mk_xor(void *const &, void *const &);
    void *prover_mk_not(void *const &);
    void *prover_mk_DC(void *const &C, void *const &D);
    void *prover_mk_HMUX(void *const &I0, void *const &I1, void *const &S);
    void *prover_mk_exor(void *const &, void *const &);
    void *prover_mk_and_exor(std::vector<void *> &exors);

    void handleQuery(void *const &queryExpr, uint32_t timeout, FILE *fout);
    void handleQuery_EQ(void *const &left, void *const &right, uint32_t timeout, FILE *fout);
    void handleQuery_Impl(void *const &left, void *const &right, uint32_t timeout, FILE *fout);
    void handleQuery_Impl(void *const &right, uint32_t timeout, FILE *fout);

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