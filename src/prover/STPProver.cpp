#include "STPProver.h"

STPProver::STPProver(/* args */)
{
    this->handle = vc_createValidityChecker();
    vc_useCryptominisat(this->handle);
    // vc_useMinisat(this->handle);
    this->bv_type = vc_bvType(handle, 2);
    this->stp_zero = vc_bvConstExprFromInt(handle, 2, 0);
    this->stp_one = vc_bvConstExprFromInt(handle, 2, 1);
    this->stp_x = vc_bvConstExprFromInt(handle, 2, 2);
    this->stp_undefined = vc_bvConstExprFromInt(handle, 2, 3);
    // Register a callback for errors
    vc_registerErrorHandler(errorHandler);
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

std::vector<Expr> &STPProver::init_exprs(std::size_t nums)
{
    std::vector<Expr>().swap(this->in_exprs);
    this->in_exprs.reserve(nums);
    std::vector<Expr>().swap(this->assert_exprs);
    this->assert_exprs.reserve(nums);
    return this->in_exprs;
}

Expr STPProver::stp_mk_variable(std::string &name)
{
    Expr var = vc_varExpr(this->handle, name.c_str(), bv_type);
    // vc_assertFormula(this->handle, vc_bvLeExpr(this->handle, var, stp_one));
    this->in_exprs.emplace_back(var);
    this->assert_exprs.push_back(vc_bvLeExpr(this->handle, var, stp_one));
    return var;
}

Expr STPProver::stp_mk_and(const Expr &A, const Expr &B)
{
    return vc_iteExpr(this->handle,
                      vc_bvLtExpr(this->handle, vc_bvXorExpr(this->handle, A, B), this->stp_undefined),
                      vc_bvAndExpr(this->handle, A, B),
                      stp_x);
}

Expr STPProver::stp_mk_and(std::vector<Expr> &exprs)
{
    Expr res = exprs[0];
    std::vector<Expr>::iterator it_ = exprs.begin() + 1;
    std::vector<Expr>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = stp_mk_and(res, *(it_++));
    }
    return res;
}

Expr STPProver::stp_mk_or(const Expr &A, const Expr &B)
{
    return vc_iteExpr(this->handle,
                      vc_bvLtExpr(this->handle, vc_bvXorExpr(this->handle, A, B), this->stp_undefined),
                      vc_bvOrExpr(this->handle, A, B),
                      stp_one);
}

Expr STPProver::stp_mk_or(std::vector<Expr> &exprs)
{
    Expr res = exprs[0];
    std::vector<Expr>::iterator it_ = exprs.begin() + 1;
    std::vector<Expr>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = stp_mk_or(res, *(it_++));
    }
    return res;
}

Expr STPProver::stp_mk_xor(const Expr &A, const Expr &B)
{
    return vc_iteExpr(this->handle,
                      vc_eqExpr(this->handle, vc_bvAndExpr(this->handle, A, B), this->stp_one),
                      stp_zero,
                      vc_iteExpr(this->handle, vc_bvGeExpr(this->handle, A, B), A, B));
}

Expr STPProver::stp_mk_xor(std::vector<Expr> &exprs)
{
    Expr res = exprs[0];
    std::vector<Expr>::iterator it_ = exprs.begin() + 1;
    std::vector<Expr>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = stp_mk_xor(res, *(it_++));
    }
    return res;
}

Expr STPProver::stp_mk_not(const Expr &A)
{
    return vc_iteExpr(this->handle,
                      vc_bvLtExpr(this->handle, A, stp_x),
                      vc_bvXorExpr(this->handle, A, stp_one),
                      stp_x);
}

Expr STPProver::stp_mk_DC(const Expr &C, const Expr &D)
{
    return vc_iteExpr(this->handle,
                      vc_bvGtExpr(this->handle, D, stp_zero),
                      stp_x,
                      C);
}

Expr STPProver::stp_mk_HMUX(const Expr &I0, const Expr &I1, const Expr &S)
{
    return vc_iteExpr(this->handle,
                      vc_bvLtExpr(this->handle, S, stp_x),
                      vc_iteExpr(this->handle, vc_bvGtExpr(this->handle, S, stp_zero), I1, I0),
                      vc_iteExpr(this->handle, vc_eqExpr(this->handle, I0, I1), I0, stp_x));
}

Expr STPProver::stp_mk_exor(const Expr &A, const Expr &B)
{
    return vc_orExpr(this->handle, vc_eqExpr(this->handle, A, stp_x), vc_eqExpr(this->handle, A, B));
}

Expr STPProver::stp_mk_and_exor(Expr *exprs, int size)
{
    return vc_andExprN(this->handle, exprs, size);
}

void STPProver::handleQuery(const Expr &queryExpr, uint32_t timeout, uint32_t max_conflicts, FILE *fout)
{
    // Print the assertions
    // printf("Assertions:\n");
    // vc_printAsserts(this->handle, 0);
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

void STPProver::handleQuery(const Expr &left, const Expr &right, uint32_t timeout, uint32_t max_conflicts, FILE *fout)
{
    this->handleQuery(vc_eqExpr(this->handle, left, right), timeout, max_conflicts, fout);
}

void STPProver::handleQuery_Impl(const Expr &left, const Expr &right, uint32_t timeout, uint32_t max_conflicts, FILE *fout) {
    this->handleQuery(vc_impliesExpr(this->handle, left, right), timeout, max_conflicts, fout);
}

void STPProver::handleQuery_Impl(const Expr &right, uint32_t timeout, uint32_t max_conflicts, FILE *fout) {
    Expr left[this->assert_exprs.size()];
    std::copy(this->assert_exprs.begin(), this->assert_exprs.end(), left);
    this->handleQuery(vc_impliesExpr(this->handle, stp_mk_and_exor(left, this->assert_exprs.size()), right), timeout, max_conflicts, fout);
}


/***************** test every operators **********************/
void STPProver::test()
{
    printf("test AND\n");
    test_AND();
    printf("test NAND\n");
    test_NAND();
    printf("test OR\n");
    test_OR();
    printf("test NOR\n");
    test_NOR();
    printf("test XOR\n");
    test_XOR();
    printf("test XNOR\n");
    test_XNOR();
    printf("test INV\n");
    test_INV();
    printf("test DC\n");
    test_DC();
    printf("test HMUX\n");
    test_HMUX();
    printf("test EXOR\n");
    test_EXOR();
}

void STPProver::test_AND()
{
    this->handleQuery(stp_mk_and(stp_zero, stp_zero), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_and(stp_zero, stp_one), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_and(stp_zero, stp_x), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_and(stp_one, stp_zero), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_and(stp_one, stp_one), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_and(stp_one, stp_x), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_and(stp_x, stp_zero), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_and(stp_x, stp_one), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_and(stp_x, stp_x), stp_x, 1000, 10000, stdout);
}
void STPProver::test_NAND()
{
}
void STPProver::test_OR()
{
    this->handleQuery(stp_mk_or(stp_zero, stp_zero), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_or(stp_zero, stp_one), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_or(stp_zero, stp_x), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_or(stp_one, stp_zero), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_or(stp_one, stp_one), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_or(stp_one, stp_x), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_or(stp_x, stp_zero), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_or(stp_x, stp_one), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_or(stp_x, stp_x), stp_x, 1000, 10000, stdout);
}
void STPProver::test_NOR() {}
void STPProver::test_XOR()
{
    this->handleQuery(stp_mk_xor(stp_zero, stp_zero), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_xor(stp_zero, stp_one), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_xor(stp_zero, stp_x), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_xor(stp_one, stp_zero), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_xor(stp_one, stp_one), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_xor(stp_one, stp_x), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_xor(stp_x, stp_zero), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_xor(stp_x, stp_one), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_xor(stp_x, stp_x), stp_x, 1000, 10000, stdout);
}
void STPProver::test_XNOR() {}
void STPProver::test_INV()
{
    this->handleQuery(stp_mk_not(stp_zero), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_not(stp_one), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_not(stp_x), stp_x, 1000, 10000, stdout);
}
void STPProver::test_DC()
{
    this->handleQuery(stp_mk_DC(stp_zero, stp_zero), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_DC(stp_zero, stp_one), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_DC(stp_zero, stp_x), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_DC(stp_one, stp_zero), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_DC(stp_one, stp_one), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_DC(stp_one, stp_x), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_DC(stp_x, stp_zero), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_DC(stp_x, stp_one), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_DC(stp_x, stp_x), stp_x, 1000, 10000, stdout);
}
void STPProver::test_HMUX()
{
    this->handleQuery(stp_mk_HMUX(stp_zero, stp_zero, stp_zero), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_zero, stp_one, stp_zero), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_zero, stp_x, stp_zero), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_one, stp_zero, stp_zero), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_one, stp_one, stp_zero), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_one, stp_x, stp_zero), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_x, stp_zero, stp_zero), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_x, stp_one, stp_zero), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_x, stp_x, stp_zero), stp_x, 1000, 10000, stdout);

    this->handleQuery(stp_mk_HMUX(stp_zero, stp_zero, stp_one), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_zero, stp_one, stp_one), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_zero, stp_x, stp_one), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_one, stp_zero, stp_one), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_one, stp_one, stp_one), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_one, stp_x, stp_one), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_x, stp_zero, stp_one), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_x, stp_one, stp_one), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_x, stp_x, stp_one), stp_x, 1000, 10000, stdout);

    this->handleQuery(stp_mk_HMUX(stp_zero, stp_zero, stp_x), stp_zero, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_zero, stp_one, stp_x), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_zero, stp_x, stp_x), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_one, stp_zero, stp_x), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_one, stp_one, stp_x), stp_one, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_one, stp_x, stp_x), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_x, stp_zero, stp_x), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_x, stp_one, stp_x), stp_x, 1000, 10000, stdout);
    this->handleQuery(stp_mk_HMUX(stp_x, stp_x, stp_x), stp_x, 1000, 10000, stdout);
}
void STPProver::test_EXOR()
{
    this->handleQuery(stp_mk_exor(stp_zero, stp_zero), 1000, 10000, stdout);
    this->handleQuery(vc_notExpr(this->handle, stp_mk_exor(stp_zero, stp_one)), 1000, 10000, stdout);
    this->handleQuery(vc_notExpr(this->handle, stp_mk_exor(stp_zero, stp_x)), 1000, 10000, stdout);
    this->handleQuery(vc_notExpr(this->handle, stp_mk_exor(stp_one, stp_zero)), 1000, 10000, stdout);
    this->handleQuery(stp_mk_exor(stp_one, stp_one), 1000, 10000, stdout);
    this->handleQuery(vc_notExpr(this->handle, stp_mk_exor(stp_one, stp_x)), 1000, 10000, stdout);
    this->handleQuery(stp_mk_exor(stp_x, stp_zero), 1000, 10000, stdout);
    this->handleQuery(stp_mk_exor(stp_x, stp_one), 1000, 10000, stdout);
    this->handleQuery(stp_mk_exor(stp_x, stp_x), 1000, 10000, stdout);
}