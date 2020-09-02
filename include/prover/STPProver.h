#ifndef _STPPROVER_H_
#define _STPPROVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "stp/c_interface.h"

class STPProver
{
private:
    VC handle;
    Type bv_type;
    std::vector<Expr> in_exprs;
    std::vector<Expr> assert_exprs;

public:
    Expr stp_zero;
    Expr stp_one;
    Expr stp_x;
    Expr stp_undefined;

    STPProver(/* args */);
    ~STPProver();

    // Error handler
    static void errorHandler(const char *err_msg);

    std::vector<Expr> &init_exprs(std::size_t nums);
    Expr stp_mk_variable(std::string &name);

    Expr stp_mk_and(const Expr &, const Expr &);
    Expr stp_mk_and(std::vector<Expr> &);
    Expr stp_mk_or(const Expr &, const Expr &);
    Expr stp_mk_or(std::vector<Expr> &);
    Expr stp_mk_xor(const Expr &, const Expr &);
    Expr stp_mk_xor(std::vector<Expr> &);
    Expr stp_mk_not(const Expr &);
    Expr stp_mk_DC(const Expr &C, const Expr &D);
    Expr stp_mk_HMUX(const Expr &I0, const Expr &I1, const Expr &S);
    Expr stp_mk_exor(const Expr &, const Expr &);
    Expr stp_mk_and_exor(Expr *exprs, int size);

    void handleQuery(const Expr &queryExpr, uint32_t timeout, uint32_t max_conflicts, FILE *fout);
    void handleQuery(const Expr &left, const Expr &right, uint32_t timeout, uint32_t max_conflicts, FILE *fout);
    void handleQuery_Impl(const Expr &left, const Expr &right, uint32_t timeout, uint32_t max_conflicts, FILE *fout);
    void handleQuery_Impl(const Expr &right, uint32_t timeout, uint32_t max_conflicts, FILE *fout);

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