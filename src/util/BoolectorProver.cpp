#include "BoolectorProver.h"

BoolectorProver::BoolectorProver(/* args */)
{
    this->btor = boolector_new();
    boolector_set_opt(this->btor, BTOR_OPT_MODEL_GEN, 1);
    boolector_set_opt(this->btor, BTOR_OPT_OUTPUT_NUMBER_FORMAT, 2);
    this->bsort = boolector_bitvec_sort(this->btor, 2);
    this->ble_zero = boolector_zero(this->btor, this->bsort);
    this->ble_one = boolector_one(this->btor, this->bsort);
    this->ble_x = boolector_unsigned_int(this->btor, 2, this->bsort);
    this->ble_undefined = boolector_unsigned_int(this->btor, 3, this->bsort);
    this->exprs.insert(this->ble_zero);
    this->exprs.insert(this->ble_one);
    this->exprs.insert(this->ble_x);
    this->exprs.insert(this->ble_undefined);
}

BoolectorProver::~BoolectorProver()
{
    this->in_exprs.clear();
    this->assert_exprs.clear();
    for (auto &item : this->exprs)
    {
        boolector_release(this->btor, item);
    }
    this->exprs.clear();
    boolector_release_sort(this->btor, this->bsort);
    // assert (boolector_get_refs(this->btor) == 0);
    boolector_delete(this->btor);
}

BoolectorNode *BoolectorProver::boolector_mk_variable(std::string &name)
{
    BoolectorNode *var = boolector_var(this->btor, this->bsort, name.c_str());
    BoolectorNode *t_assert = boolector_ult(this->btor, var, this->ble_x);
    boolector_assert(this->btor, t_assert);
    this->exprs.insert(var);
    this->assert_exprs.emplace_back(t_assert);
    this->in_exprs[name] = var;
    return var;
}

BoolectorNode *BoolectorProver::boolector_mk_and(BoolectorNode *A, BoolectorNode *B)
{
    BoolectorNode *t_xor = boolector_xor(this->btor, A, B);
    BoolectorNode *t_ult = boolector_ult(this->btor, t_xor, this->ble_undefined);
    BoolectorNode *res = boolector_cond(this->btor, t_ult, boolector_and(this->btor, A, B), this->ble_x);
    boolector_release(this->btor, t_xor);
    boolector_release(this->btor, t_ult);
    this->exprs.insert(res);
    return res;
}

BoolectorNode *BoolectorProver::boolector_mk_and(std::vector<BoolectorNode *> &exprs)
{
    if (exprs.empty())
    {
        return NULL;
    }
    BoolectorNode *res = exprs[0];
    std::vector<BoolectorNode *>::iterator it_ = exprs.begin() + 1;
    std::vector<BoolectorNode *>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = boolector_mk_and(res, *(it_++));
        this->exprs.insert(res);
    }
    return res;
}

BoolectorNode *BoolectorProver::boolector_mk_or(BoolectorNode *A, BoolectorNode *B)
{
    BoolectorNode *t_xor = boolector_xor(this->btor, A, B);
    BoolectorNode *t_ult = boolector_ult(this->btor, t_xor, this->ble_undefined);
    BoolectorNode *res = boolector_cond(this->btor, t_ult, boolector_or(this->btor, A, B), this->ble_one);
    boolector_release(this->btor, t_xor);
    boolector_release(this->btor, t_ult);
    this->exprs.insert(res);
    return res;
}

BoolectorNode *BoolectorProver::boolector_mk_or(std::vector<BoolectorNode *> &exprs)
{
    if (exprs.empty())
    {
        return NULL;
    }
    BoolectorNode *res = exprs[0];
    std::vector<BoolectorNode *>::iterator it_ = exprs.begin() + 1;
    std::vector<BoolectorNode *>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = boolector_mk_or(res, *(it_++));
        this->exprs.insert(res);
    }
    return res;
}

BoolectorNode *BoolectorProver::boolector_mk_xor(BoolectorNode *A, BoolectorNode *B)
{
    BoolectorNode *t_and = boolector_xor(this->btor, A, B);
    BoolectorNode *t_eq = boolector_eq(this->btor, t_and, this->ble_one);
    BoolectorNode *t_uge = boolector_ugte(this->btor, A, B);
    BoolectorNode *t_cond = boolector_cond(this->btor, t_uge, A, B);
    BoolectorNode *res = boolector_cond(this->btor, t_eq, this->ble_zero, t_cond);
    boolector_release(this->btor, t_and);
    boolector_release(this->btor, t_eq);
    boolector_release(this->btor, t_uge);
    this->exprs.insert(t_cond);
    this->exprs.insert(res);
    return res;
}

BoolectorNode *BoolectorProver::boolector_mk_xor(std::vector<BoolectorNode *> &exprs)
{
    if (exprs.empty())
    {
        return NULL;
    }
    BoolectorNode *res = exprs[0];
    std::vector<BoolectorNode *>::iterator it_ = exprs.begin() + 1;
    std::vector<BoolectorNode *>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = boolector_mk_xor(res, *(it_++));
        this->exprs.insert(res);
    }
    return res;
}

BoolectorNode *BoolectorProver::boolector_mk_not(BoolectorNode *A)
{
    BoolectorNode *t_ult = boolector_ult(this->btor, A, this->ble_x);
    BoolectorNode *t_xor = boolector_xor(this->btor, A, this->ble_one);
    BoolectorNode *res = boolector_cond(this->btor, t_ult, t_xor, this->ble_x);
    boolector_release(this->btor, t_ult);
    boolector_release(this->btor, t_xor);
    this->exprs.insert(res);
    return res;
}

BoolectorNode *BoolectorProver::boolector_mk_DC(BoolectorNode *C, BoolectorNode *D)
{
    BoolectorNode *t_uge = boolector_ugt(this->btor, D, this->ble_zero);
    BoolectorNode *res = boolector_cond(this->btor, t_uge, this->ble_x, C);
    boolector_release(this->btor, t_uge);
    this->exprs.insert(res);
    return res;
}

BoolectorNode *BoolectorProver::boolector_mk_HMUX(BoolectorNode *I0, BoolectorNode *I1, BoolectorNode *S)
{
    BoolectorNode *t_ult = boolector_ult(this->btor, S, this->ble_x);
    BoolectorNode *t_ugt = boolector_ugt(this->btor, S, this->ble_zero);
    BoolectorNode *t_cond_gt = boolector_cond(this->btor, t_ugt, I1, I0);
    BoolectorNode *t_eq = boolector_eq(this->btor, I0, I1);
    BoolectorNode *t_cond_eq = boolector_cond(this->btor, t_eq, I0, this->ble_x);
    BoolectorNode *res = boolector_cond(this->btor, t_ult, t_cond_gt, t_cond_eq);
    boolector_release(this->btor, t_ult);
    boolector_release(this->btor, t_ugt);
    boolector_release(this->btor, t_eq);
    this->exprs.insert(t_cond_gt);
    this->exprs.insert(t_cond_eq);
    this->exprs.insert(res);
    return res;
}

BoolectorNode *BoolectorProver::boolector_mk_exor(BoolectorNode *A, BoolectorNode *B)
{
    BoolectorNode *t_eq_A_x = boolector_eq(this->btor, A, this->ble_x);
    BoolectorNode *t_eq_A_B = boolector_eq(this->btor, A, B);
    BoolectorNode *res = boolector_or(this->btor, t_eq_A_x, t_eq_A_B);
    this->exprs.insert(t_eq_A_x);
    this->exprs.insert(t_eq_A_B);
    this->exprs.insert(res);
    return res;
}

BoolectorNode *BoolectorProver::boolector_mk_and_exor(std::vector<BoolectorNode *> &exprs) {
    if (exprs.empty())
    {
        return NULL;
    }
    BoolectorNode *res = exprs[0];
    std::vector<BoolectorNode *>::iterator it_ = exprs.begin() + 1;
    std::vector<BoolectorNode *>::iterator it_end = exprs.end();
    while (it_ != it_end)
    {
        res = boolector_and(this->btor, res, *(it_++));
        this->exprs.insert(res);
    }
    return res;
}


void BoolectorProver::handleQuery(BoolectorNode *queryExpr, uint32_t timeout, FILE *fout)
{
    BoolectorNode *formula = boolector_not(this->btor, queryExpr);
    boolector_assert(this->btor, formula);
    int result = boolector_limited_sat(this->btor, -1, -1);
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
            const char *assign = boolector_bv_assignment(this->btor, pi.second);
            fprintf(fout, "%s %s\n", pi.first.c_str(), assign);
            boolector_free_bv_assignment(this->btor, assign);
        }
        break;
    default:
        printf("Unhandled error\n");
    }
    boolector_release(this->btor, formula);
}

void BoolectorProver::handleQuery_EQ(BoolectorNode *left, BoolectorNode *right, uint32_t timeout, FILE *fout)
{
    BoolectorNode *t_eq = boolector_eq(this->btor, left, right);
    this->handleQuery(t_eq, timeout, fout);
    boolector_release(this->btor, t_eq);
}

void BoolectorProver::handleQuery_Impl(BoolectorNode *left, BoolectorNode *right, uint32_t timeout, FILE *fout)
{
    BoolectorNode *t_eq = boolector_implies(this->btor, left, right);
    this->handleQuery(t_eq, timeout, fout);
    boolector_release(this->btor, t_eq);
}

void BoolectorProver::handleQuery_Impl(BoolectorNode *right, uint32_t timeout, FILE *fout)
{
    BoolectorNode *t_eq = boolector_implies(this->btor, boolector_mk_and_exor(this->assert_exprs), right);
    this->handleQuery(t_eq, timeout, fout);
    boolector_release(this->btor, t_eq);
}

/***************** test every operators **********************/
void BoolectorProver::test()
{
    boolector_set_opt(this->btor, BTOR_OPT_INCREMENTAL, 1);
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

void BoolectorProver::test_AND()
{
    this->handleQuery_EQ(boolector_mk_and(this->ble_zero, this->ble_zero), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_and(this->ble_zero, this->ble_one), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_and(this->ble_zero, this->ble_x), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_and(this->ble_one, this->ble_zero), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_and(this->ble_one, this->ble_one), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_and(this->ble_one, this->ble_x), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_and(this->ble_x, this->ble_zero), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_and(this->ble_x, this->ble_one), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_and(this->ble_x, this->ble_x), this->ble_x, 10, stdout);
}
void BoolectorProver::test_NAND()
{
}
void BoolectorProver::test_OR()
{
    this->handleQuery_EQ(boolector_mk_or(this->ble_zero, this->ble_zero), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_or(this->ble_zero, this->ble_one), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_or(this->ble_zero, this->ble_x), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_or(this->ble_one, this->ble_zero), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_or(this->ble_one, this->ble_one), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_or(this->ble_one, this->ble_x), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_or(this->ble_x, this->ble_zero), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_or(this->ble_x, this->ble_one), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_or(this->ble_x, this->ble_x), this->ble_x, 10, stdout);
}
void BoolectorProver::test_NOR() {}
void BoolectorProver::test_XOR()
{
    this->handleQuery_EQ(boolector_mk_xor(this->ble_zero, this->ble_zero), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_xor(this->ble_zero, this->ble_one), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_xor(this->ble_zero, this->ble_x), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_xor(this->ble_one, this->ble_zero), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_xor(this->ble_one, this->ble_one), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_xor(this->ble_one, this->ble_x), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_xor(this->ble_x, this->ble_zero), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_xor(this->ble_x, this->ble_one), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_xor(this->ble_x, this->ble_x), this->ble_x, 10, stdout);
}
void BoolectorProver::test_XNOR() {}
void BoolectorProver::test_INV()
{
    this->handleQuery_EQ(boolector_mk_not(this->ble_zero), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_not(this->ble_one), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_not(this->ble_x), this->ble_x, 10, stdout);
}
void BoolectorProver::test_DC()
{
    this->handleQuery_EQ(boolector_mk_DC(this->ble_zero, this->ble_zero), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_DC(this->ble_zero, this->ble_one), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_DC(this->ble_zero, this->ble_x), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_DC(this->ble_one, this->ble_zero), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_DC(this->ble_one, this->ble_one), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_DC(this->ble_one, this->ble_x), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_DC(this->ble_x, this->ble_zero), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_DC(this->ble_x, this->ble_one), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_DC(this->ble_x, this->ble_x), this->ble_x, 10, stdout);
}
void BoolectorProver::test_HMUX()
{
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_zero, this->ble_zero, this->ble_zero), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_zero, this->ble_one, this->ble_zero), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_zero, this->ble_x, this->ble_zero), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_one, this->ble_zero, this->ble_zero), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_one, this->ble_one, this->ble_zero), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_one, this->ble_x, this->ble_zero), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_x, this->ble_zero, this->ble_zero), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_x, this->ble_one, this->ble_zero), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_x, this->ble_x, this->ble_zero), this->ble_x, 10, stdout);

    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_zero, this->ble_zero, this->ble_one), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_zero, this->ble_one, this->ble_one), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_zero, this->ble_x, this->ble_one), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_one, this->ble_zero, this->ble_one), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_one, this->ble_one, this->ble_one), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_one, this->ble_x, this->ble_one), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_x, this->ble_zero, this->ble_one), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_x, this->ble_one, this->ble_one), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_x, this->ble_x, this->ble_one), this->ble_x, 10, stdout);

    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_zero, this->ble_zero, this->ble_x), this->ble_zero, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_zero, this->ble_one, this->ble_x), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_zero, this->ble_x, this->ble_x), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_one, this->ble_zero, this->ble_x), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_one, this->ble_one, this->ble_x), this->ble_one, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_one, this->ble_x, this->ble_x), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_x, this->ble_zero, this->ble_x), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_x, this->ble_one, this->ble_x), this->ble_x, 10, stdout);
    this->handleQuery_EQ(boolector_mk_HMUX(this->ble_x, this->ble_x, this->ble_x), this->ble_x, 10, stdout);
}
void BoolectorProver::test_EXOR()
{
    this->handleQuery(boolector_mk_exor(this->ble_zero, this->ble_zero), 10, stdout);
    this->handleQuery(boolector_not(this->btor, boolector_mk_exor(this->ble_zero, this->ble_one)), 10, stdout);
    this->handleQuery(boolector_not(this->btor, boolector_mk_exor(this->ble_zero, this->ble_x)), 10, stdout);
    this->handleQuery(boolector_not(this->btor, boolector_mk_exor(this->ble_one, this->ble_zero)), 10, stdout);
    this->handleQuery(boolector_mk_exor(this->ble_one, this->ble_one), 10, stdout);
    this->handleQuery(boolector_not(this->btor, boolector_mk_exor(this->ble_one, this->ble_x)), 10, stdout);
    this->handleQuery(boolector_mk_exor(this->ble_x, this->ble_zero), 10, stdout);
    this->handleQuery(boolector_mk_exor(this->ble_x, this->ble_one), 10, stdout);
    this->handleQuery(boolector_mk_exor(this->ble_x, this->ble_x), 10, stdout);
}