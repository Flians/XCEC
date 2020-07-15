#ifndef _STPPROVER_H_
#define _STPPROVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <stp/c_interface.h>

class STPProver
{
private:
    std::vector<Expr> exprs;
public:
    VC handle;
    Type bv_type;
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
    Expr stp_mk_HMUX(const Expr &S, const Expr &I0, const Expr &I1);
    Expr stp_mk_exor(const Expr &, const Expr &);

    void handleQuery(Expr queryExpr, FILE *fout);
};

#endif