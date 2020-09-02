#include "BoolectorProver.h"

BoolectorProver::BoolectorProver()
{
    this->handle = boolector_new();
    boolector_set_opt((Btor *)this->handle, BTOR_OPT_MODEL_GEN, 1);
    boolector_set_opt((Btor *)this->handle, BTOR_OPT_OUTPUT_NUMBER_FORMAT, 2);
    boolector_set_opt((Btor *)this->handle, BTOR_OPT_AUTO_CLEANUP, 1);
    this->bv_sort = boolector_bitvec_sort((Btor *)this->handle, 2);
    this->prover_zero = boolector_zero((Btor *)this->handle, this->bv_sort);
    this->prover_one = boolector_one((Btor *)this->handle, this->bv_sort);
    this->prover_x = boolector_unsigned_int((Btor *)this->handle, 2, this->bv_sort);
    this->prover_undefined = boolector_unsigned_int((Btor *)this->handle, 3, this->bv_sort);
    this->exprs.insert(this->prover_zero);
    this->exprs.insert(this->prover_one);
    this->exprs.insert(this->prover_x);
    this->exprs.insert(this->prover_undefined);
}

BoolectorProver::BoolectorProver(size_t len)
{
    new (this) BoolectorProver();
    this->assert_exprs.reserve(len);
    this->in_exprs.reserve(len);
}

BoolectorProver::~BoolectorProver()
{
    for (auto &item : this->exprs)
    {
        boolector_release((Btor *)this->handle, (BoolectorNode *)item);
    }
    boolector_release_sort((Btor *)this->handle, this->bv_sort);
    // assert (boolector_get_refs(this->handle) == 0);
    boolector_delete((Btor *)this->handle);
    printf("The BoolectorProver is destroyed!\n");
}

void *BoolectorProver::prover_mk_variable(const std::string &name)
{
    BoolectorNode *var = boolector_var((Btor *)this->handle, this->bv_sort, name.c_str());
    BoolectorNode *t_assert = boolector_ult((Btor *)this->handle, var, (BoolectorNode *)this->prover_x);
    // boolector_assert((Btor *)this->handle, t_assert);
    this->exprs.insert(var);
    this->assert_exprs.emplace_back(t_assert);
    this->in_exprs.emplace_back(var);
    return var;
}

void *BoolectorProver::prover_mk_and(void *const &A, void *const &B)
{
    BoolectorNode *t_xor = boolector_xor((Btor *)this->handle, (BoolectorNode *)A, (BoolectorNode *)B);
    BoolectorNode *t_ult = boolector_ult((Btor *)this->handle, t_xor, (BoolectorNode *)this->prover_undefined);
    BoolectorNode *res = boolector_cond((Btor *)this->handle, t_ult, boolector_and((Btor *)this->handle, (BoolectorNode *)A, (BoolectorNode *)B), (BoolectorNode *)this->prover_x);
    boolector_release((Btor *)this->handle, t_xor);
    boolector_release((Btor *)this->handle, t_ult);
    this->exprs.insert(res);
    return res;
}

void *BoolectorProver::prover_mk_or(void *const &A, void *const &B)
{
    BoolectorNode *t_xor = boolector_xor((Btor *)this->handle, (BoolectorNode *)A, (BoolectorNode *)B);
    BoolectorNode *t_ult = boolector_ult((Btor *)this->handle, t_xor, (BoolectorNode *)this->prover_undefined);
    BoolectorNode *res = boolector_cond((Btor *)this->handle, t_ult, boolector_or((Btor *)this->handle, (BoolectorNode *)A, (BoolectorNode *)B), (BoolectorNode *)this->prover_one);
    boolector_release((Btor *)this->handle, t_xor);
    boolector_release((Btor *)this->handle, t_ult);
    this->exprs.insert(res);
    return res;
}

void *BoolectorProver::prover_mk_xor(void *const &A, void *const &B)
{
    BoolectorNode *t_and = boolector_and((Btor *)this->handle, (BoolectorNode *)A, (BoolectorNode *)B);
    BoolectorNode *t_eq = boolector_eq((Btor *)this->handle, t_and, (BoolectorNode *)this->prover_one);
    BoolectorNode *t_uge = boolector_ugte((Btor *)this->handle, (BoolectorNode *)A, (BoolectorNode *)B);
    BoolectorNode *t_cond = boolector_cond((Btor *)this->handle, t_uge, (BoolectorNode *)A, (BoolectorNode *)B);
    BoolectorNode *res = boolector_cond((Btor *)this->handle, t_eq, (BoolectorNode *)this->prover_zero, t_cond);
    boolector_release((Btor *)this->handle, t_and);
    boolector_release((Btor *)this->handle, t_eq);
    boolector_release((Btor *)this->handle, t_uge);
    this->exprs.insert(t_cond);
    this->exprs.insert(res);
    return res;
}

void *BoolectorProver::prover_mk_not(void *const &A)
{
    BoolectorNode *t_ult = boolector_ult((Btor *)this->handle, (BoolectorNode *)A, (BoolectorNode *)this->prover_x);
    BoolectorNode *t_xor = boolector_xor((Btor *)this->handle, (BoolectorNode *)A, (BoolectorNode *)this->prover_one);
    BoolectorNode *res = boolector_cond((Btor *)this->handle, t_ult, t_xor, (BoolectorNode *)this->prover_x);
    boolector_release((Btor *)this->handle, t_ult);
    boolector_release((Btor *)this->handle, t_xor);
    this->exprs.insert(res);
    return res;
}

void *BoolectorProver::prover_mk_DC(void *const &C, void *const &D)
{
    BoolectorNode *t_uge = boolector_ugt((Btor *)this->handle, (BoolectorNode *)D, (BoolectorNode *)this->prover_zero);
    BoolectorNode *res = boolector_cond((Btor *)this->handle, t_uge, (BoolectorNode *)this->prover_x, (BoolectorNode *)C);
    boolector_release((Btor *)this->handle, t_uge);
    this->exprs.insert(res);
    return res;
}

void *BoolectorProver::prover_mk_HMUX(void *const &I0, void *const &I1, void *const &S)
{
    BoolectorNode *t_ult = boolector_ult((Btor *)this->handle, (BoolectorNode *)S, (BoolectorNode *)this->prover_x);
    BoolectorNode *t_ugt = boolector_ugt((Btor *)this->handle, (BoolectorNode *)S, (BoolectorNode *)this->prover_zero);
    BoolectorNode *t_cond_gt = boolector_cond((Btor *)this->handle, t_ugt, (BoolectorNode *)I1, (BoolectorNode *)I0);
    BoolectorNode *t_eq = boolector_eq((Btor *)this->handle, (BoolectorNode *)I0, (BoolectorNode *)I1);
    BoolectorNode *t_cond_eq = boolector_cond((Btor *)this->handle, t_eq, (BoolectorNode *)I0, (BoolectorNode *)this->prover_x);
    BoolectorNode *res = boolector_cond((Btor *)this->handle, t_ult, t_cond_gt, t_cond_eq);
    boolector_release((Btor *)this->handle, t_ult);
    boolector_release((Btor *)this->handle, t_ugt);
    boolector_release((Btor *)this->handle, t_eq);
    this->exprs.insert(t_cond_gt);
    this->exprs.insert(t_cond_eq);
    this->exprs.insert(res);
    return res;
}

void *BoolectorProver::prover_mk_exor(void *const &A, void *const &B)
{
    BoolectorNode *t_eq_A_x = boolector_eq((Btor *)this->handle, (BoolectorNode *)A, (BoolectorNode *)this->prover_x);
    BoolectorNode *t_eq_A_B = boolector_eq((Btor *)this->handle, (BoolectorNode *)A, (BoolectorNode *)B);
    BoolectorNode *res = boolector_or((Btor *)this->handle, t_eq_A_x, t_eq_A_B);
    this->exprs.insert(t_eq_A_x);
    this->exprs.insert(t_eq_A_B);
    this->exprs.insert(res);
    return res;
}

void *BoolectorProver::prover_mk_and_exor(std::vector<void *> &exors)
{
    if (exors.empty())
    {
        return NULL;
    }
    void *res = exors[0];
    std::vector<void *>::iterator it_ = exors.begin() + 1;
    std::vector<void *>::iterator it_end = exors.end();
    while (it_ != it_end)
    {
        res = boolector_and((Btor *)this->handle, (BoolectorNode *)res, (BoolectorNode *)*(it_++));
        this->exprs.insert(res);
    }
    return res;
}

void BoolectorProver::handleQuery(void *const &queryExpr, uint32_t timeout, uint32_t max_conflicts, FILE *fout)
{
    BoolectorNode *formula = boolector_not((Btor *)this->handle, (BoolectorNode *)queryExpr);
    boolector_assert((Btor *)this->handle, formula);
    int result = boolector_limited_sat((Btor *)this->handle, timeout, max_conflicts);
    switch (result)
    {
    case BOOLECTOR_UNKNOWN:
        printf("Could not answer query\n");
    case BOOLECTOR_UNSAT:
        fprintf(fout, "EQ\n");
        break;
    case BOOLECTOR_SAT:
        fprintf(fout, "NEQ\n");
        for (auto &pi : this->in_exprs)
        {
            const char *assign = boolector_bv_assignment((Btor *)this->handle, (BoolectorNode *)pi);
            const char *symbol = boolector_get_symbol((Btor *)this->handle, (BoolectorNode *)pi);
            fprintf(fout, "%s %s\n", symbol, assign);
            boolector_free_bv_assignment((Btor *)this->handle, assign);
        }
        break;
    default:
        printf("Unhandled error\n");
    }
    boolector_release((Btor *)this->handle, formula);
}

void BoolectorProver::handleQuery_EQ(void *const &left, void *const &right, uint32_t timeout, uint32_t max_conflicts, FILE *fout)
{
    BoolectorNode *t_eq = boolector_eq((Btor *)this->handle, (BoolectorNode *)left, (BoolectorNode *)right);
    this->handleQuery(t_eq, timeout, max_conflicts, fout);
    boolector_release((Btor *)this->handle, t_eq);
}

void BoolectorProver::handleQuery_Impl(void *const &left, void *const &right, uint32_t timeout, uint32_t max_conflicts, FILE *fout)
{
    BoolectorNode *t_eq = boolector_implies((Btor *)this->handle, (BoolectorNode *)left, (BoolectorNode *)right);
    this->handleQuery(t_eq, timeout, max_conflicts, fout);
    boolector_release((Btor *)this->handle, t_eq);
}

void BoolectorProver::handleQuery_Impl(void *const &right, uint32_t timeout, uint32_t max_conflicts, FILE *fout)
{
    BoolectorNode *t_eq = boolector_implies((Btor *)this->handle, (BoolectorNode *)prover_mk_and_exor(this->assert_exprs), (BoolectorNode *)right);
    this->handleQuery(t_eq, timeout, max_conflicts, fout);
    boolector_release((Btor *)this->handle, t_eq);
}

/***************** test every operators **********************/
void BoolectorProver::test()
{
    // boolector_set_opt((Btor *)this->handle, BTOR_OPT_INCREMENTAL, 1);
    Prover::test();
}

void BoolectorProver::test_NAND() {}
void BoolectorProver::test_NOR() {}
void BoolectorProver::test_XNOR() {}

void BoolectorProver::test_EXOR()
{
    this->handleQuery(prover_mk_exor(this->prover_zero, this->prover_zero), 1000, 10000, stdout);
    this->handleQuery(boolector_not((Btor *)this->handle, (BoolectorNode *)prover_mk_exor(this->prover_zero, this->prover_one)), 0, 10000, stdout);
    this->handleQuery(boolector_not((Btor *)this->handle, (BoolectorNode *)prover_mk_exor(this->prover_zero, this->prover_x)), 0, 10000, stdout);
    this->handleQuery(boolector_not((Btor *)this->handle, (BoolectorNode *)prover_mk_exor(this->prover_one, this->prover_zero)), 0, 10000, stdout);
    this->handleQuery(prover_mk_exor(this->prover_one, this->prover_one), 1000, 10000, stdout);
    this->handleQuery(boolector_not((Btor *)this->handle, (BoolectorNode *)prover_mk_exor(this->prover_one, this->prover_x)), 0, 10000, stdout);
    this->handleQuery(prover_mk_exor(this->prover_x, this->prover_zero), 1000, 10000, stdout);
    this->handleQuery(prover_mk_exor(this->prover_x, this->prover_one), 1000, 10000, stdout);
    this->handleQuery(prover_mk_exor(this->prover_x, this->prover_x), 1000, 10000, stdout);
}