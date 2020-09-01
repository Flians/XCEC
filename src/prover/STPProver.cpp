#include "STPProver.h"

STPProver::STPProver(/* args */)
{
    
    this->handle = vc_createValidityChecker();
    vc_useCryptominisat(this->handle);
    // vc_useMinisat(this->handle);
    this->bv_sort = vc_bvType(handle, 2);
    this->prover_zero = vc_bvConstExprFromInt(handle, 2, 0);
    this->prover_one = vc_bvConstExprFromInt(handle, 2, 1);
    this->prover_x = vc_bvConstExprFromInt(handle, 2, 2);
    this->prover_undefined = vc_bvConstExprFromInt(handle, 2, 3);
    // Register a callback for errors
    vc_registerErrorHandler(errorHandler);
}

STPProver::STPProver(size_t len)
{
    new (this) STPProver();
    this->assert_exprs.reserve(len);
    this->in_exprs.reserve(len);
}

STPProver::~STPProver()
{
    // Clean up
    vc_Destroy(handle);
    std::vector<Expr>().swap(this->in_exprs);
    std::vector<Expr>().swap(this->assert_exprs);
}

// Error handler
void STPProver::errorHandler(const char *err_msg)
{
    printf("Error: %s\n", err_msg);
    exit(1);
}

void * STPProver::prover_mk_variable(const std::string &name)
{
    Expr var = vc_varExpr(this->handle, name.c_str(), bv_sort);
    // vc_assertFormula(this->handle, vc_bvLeExpr(this->handle, var, stp_one));
    this->in_exprs.emplace_back(var);
    this->assert_exprs.push_back(vc_bvLeExpr(this->handle, var, stp_one));
    return var;
}

void * STPProver::prover_mk_and(const void *&A, const void *&B)
{
    return vc_iteExpr(this->handle,
                      vc_bvLtExpr(this->handle, vc_bvXorExpr(this->handle, A, B), this->stp_undefined),
                      vc_bvAndExpr(this->handle, A, B),
                      stp_x);
}

void *STPProver::prover_mk_and(std::vector<Expr> &exprs)
{
    Expr res = exprs[0];
    std::vector<Expr>::iterator it_ = exprs.begin() + 1;
    std::vector<Expr>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = prover_mk_and(res, *(it_++));
    }
    return res;
}

void *STPProver::prover_mk_or(const void *&A, const void *&B)
{
    return vc_iteExpr(this->handle,
                      vc_bvLtExpr(this->handle, vc_bvXorExpr(this->handle, A, B), this->stp_undefined),
                      vc_bvOrExpr(this->handle, A, B),
                      stp_one);
}

void *STPProver::prover_mk_or(std::vector<Expr> &exprs)
{
    Expr res = exprs[0];
    std::vector<Expr>::iterator it_ = exprs.begin() + 1;
    std::vector<Expr>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = prover_mk_or(res, *(it_++));
    }
    return res;
}

void *STPProver::prover_mk_xor(const void *&A, const void *&B)
{
    return vc_iteExpr(this->handle,
                      vc_eqExpr(this->handle, vc_bvAndExpr(this->handle, A, B), this->stp_one),
                      stp_zero,
                      vc_iteExpr(this->handle, vc_bvGeExpr(this->handle, A, B), A, B));
}

void *STPProver::prover_mk_xor(std::vector<Expr> &exprs)
{
    Expr res = exprs[0];
    std::vector<Expr>::iterator it_ = exprs.begin() + 1;
    std::vector<Expr>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = prover_mk_xor(res, *(it_++));
    }
    return res;
}

void *STPProver::prover_mk_not(const void *&A)
{
    return vc_iteExpr(this->handle,
                      vc_bvLtExpr(this->handle, A, stp_x),
                      vc_bvXorExpr(this->handle, A, stp_one),
                      stp_x);
}

void *STPProver::prover_mk_DC(const void *&C, const void *&D)
{
    return vc_iteExpr(this->handle,
                      vc_bvGtExpr(this->handle, D, stp_zero),
                      stp_x,
                      C);
}

void *STPProver::prover_mk_HMUX(const void *&I0, const void *&I1, const void *&S)
{
    return vc_iteExpr(this->handle,
                      vc_bvLtExpr(this->handle, S, stp_x),
                      vc_iteExpr(this->handle, vc_bvGtExpr(this->handle, S, stp_zero), I1, I0),
                      vc_iteExpr(this->handle, vc_eqExpr(this->handle, I0, I1), I0, stp_x));
}

void *STPProver::prover_mk_exor(const void *&A, const void *&B)
{
    return vc_orExpr(this->handle, vc_eqExpr(this->handle, A, stp_x), vc_eqExpr(this->handle, A, B));
}

void *STPProver::prover_mk_and_exor(void **exprs, int size)
{
    return vc_andExprN(this->handle, exprs, size);
}

void STPProver::handleQuery(const void *&queryExpr, uint32_t timeout, FILE *fout)
{
    // Print the assertions
    // printf("Assertions:\n");
    // vc_printAsserts(this->handle, 0);
    int max_conflicts = -1;
    // int result = vc_query(this->handle, queryExpr);
    int result = vc_query_with_timeout(this->handle, queryExpr, max_conflicts, timeout);
    // printf("Query:\n");
    // vc_printQuery(this->handle);
    switch (result)
    {
    case 3:
        printf("Timeout.\n");
    case 1:
        fprintf(fout, "EQ\n");
        break;
    case 2:
        printf("Could not answer query\n");
    case 0:
        fprintf(fout, "NEQ\n");
        // print counter example
        // printf("Counter example:\n");
        // vc_printCounterExample(this->handle);
        for (auto &pi : this->in_exprs)
        {
            fprintf(fout, "%s %d\n", exprString(pi), getBVUnsigned(vc_getCounterExample(this->handle, pi)));
        }
        break;
    default:
        printf("Unhandled error\n");
    }
    vc_DeleteExpr(queryExpr);
}

void STPProver::handleQuery(const void *&left, const void *&right, uint32_t timeout, FILE *fout)
{
    this->handleQuery(vc_eqExpr(this->handle, left, right), timeout, fout);
}

void STPProver::handleQuery_Impl(const void *&left, const void *&right, uint32_t timeout, FILE *fout) {
    this->handleQuery(vc_impliesExpr(this->handle, left, right), timeout, fout);
}

void STPProver::handleQuery_Impl(const void *&right, uint32_t timeout, FILE *fout) {
    Expr left[this->assert_exprs.size()];
    std::copy(this->assert_exprs.begin(), this->assert_exprs.end(), left);
    this->handleQuery(vc_impliesExpr(this->handle, prover_mk_and_exor(left, this->assert_exprs.size()), right), timeout, fout);
}


/***************** test every operators **********************/
void STPProver::test_NAND() {}
void STPProver::test_NOR() {}
void STPProver::test_XNOR() {}

void STPProver::test_EXOR()
{
    this->handleQuery(prover_mk_exor(stp_zero, stp_zero), 10, stdout);
    this->handleQuery(vc_notExpr(this->handle, prover_mk_exor(stp_zero, prover_one)), 10, stdout);
    this->handleQuery(vc_notExpr(this->handle, prover_mk_exor(stp_zero, prover_x)), 10, stdout);
    this->handleQuery(vc_notExpr(this->handle, prover_mk_exor(prover_one, stp_zero)), 10, stdout);
    this->handleQuery(prover_mk_exor(prover_one, prover_one), 10, stdout);
    this->handleQuery(vc_notExpr(this->handle, prover_mk_exor(prover_one, prover_x)), 10, stdout);
    this->handleQuery(prover_mk_exor(prover_x, stp_zero), 10, stdout);
    this->handleQuery(prover_mk_exor(prover_x, prover_one), 10, stdout);
    this->handleQuery(prover_mk_exor(prover_x, prover_x), 10, stdout);
}