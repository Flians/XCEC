#include "BoolectorProver.h"

BoolectorProver::BoolectorProver()
{
    this->handle = boolector_new();
    boolector_set_opt(this->handle, BTOR_OPT_MODEL_GEN, 1);
    boolector_set_opt(this->handle, BTOR_OPT_OUTPUT_NUMBER_FORMAT, 2);
    this->bv_sort = boolector_bitvec_sort(this->handle, 2);
    this->prover_zero.reset(boolector_zero(this->handle, this->bv_sort));
    this->prover_one.reset(boolector_one(this->handle, this->bv_sort));
    this->prover_x.reset(boolector_unsigned_int(this->handle, 2, this->bv_sort));
    this->prover_undefined.reset(boolector_unsigned_int(this->handle, 3, this->bv_sort));
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
        boolector_release(this->handle, static_cast<BoolectorNode*>(item.get()));
    }
    boolector_release_sort(this->handle, this->bv_sort);
    // assert (boolector_get_refs(this->handle) == 0);
    boolector_delete(this->handle);
    printf("The BoolectorProver is destroyed!\n");
}

std::shared_ptr<void> BoolectorProver::prover_mk_variable(const std::string &name)
{
    std::shared_ptr<void> var(boolector_var(this->handle, this->bv_sort, name.c_str()));
    std::shared_ptr<void> t_assert(boolector_ult(this->handle, static_cast<BoolectorNode*>(var.get()), static_cast<BoolectorNode*>(this->prover_x.get())));
    // boolector_assert(this->handle, t_assert);
    this->exprs.insert(var);
    this->assert_exprs.emplace_back(t_assert);
    this->in_exprs.emplace_back(var);
    return var;
}

std::shared_ptr<void> BoolectorProver::prover_mk_and(std::shared_ptr<void> const &A, std::shared_ptr<void> const &B)
{
    BoolectorNode *t_xor = boolector_xor(this->handle, static_cast<BoolectorNode*>(A.get()), static_cast<BoolectorNode*>(B.get()));
    BoolectorNode *t_ult = boolector_ult(this->handle, t_xor, static_cast<BoolectorNode*>(this->prover_undefined.get()));
    std::shared_ptr<void> res(boolector_cond(this->handle, 
                                    t_ult, 
                                    boolector_and(this->handle, static_cast<BoolectorNode*>(A.get()), static_cast<BoolectorNode*>(B.get())), 
                                    static_cast<BoolectorNode*>(this->prover_x.get())));
    boolector_release(this->handle, t_xor);
    boolector_release(this->handle, t_ult);
    this->exprs.insert(res);
    return res;
}

std::shared_ptr<void> BoolectorProver::prover_mk_or(std::shared_ptr<void> const &A, std::shared_ptr<void> const &B)
{
    BoolectorNode *t_xor = boolector_xor(this->handle, static_cast<BoolectorNode*>(A.get()), static_cast<BoolectorNode*>(B.get()));
    BoolectorNode *t_ult = boolector_ult(this->handle, t_xor, static_cast<BoolectorNode*>(this->prover_undefined.get()));
    std::shared_ptr<void> res(boolector_cond(this->handle, 
                            t_ult, 
                            boolector_or(this->handle, static_cast<BoolectorNode*>(A.get()), static_cast<BoolectorNode*>(B.get())), 
                            static_cast<BoolectorNode*>(this->prover_one.get())));
    boolector_release(this->handle, t_xor);
    boolector_release(this->handle, t_ult);
    this->exprs.insert(res);
    return res;
}

std::shared_ptr<void> BoolectorProver::prover_mk_xor(std::shared_ptr<void> const &A, std::shared_ptr<void> const &B)
{
    BoolectorNode *t_and = boolector_and(this->handle, static_cast<BoolectorNode*>(A.get()), static_cast<BoolectorNode*>(B.get()));
    BoolectorNode *t_eq = boolector_eq(this->handle, t_and, static_cast<BoolectorNode*>(this->prover_one.get()));
    BoolectorNode *t_uge = boolector_ugte(this->handle, static_cast<BoolectorNode*>(A.get()), static_cast<BoolectorNode*>(B.get()));
    std::shared_ptr<void> t_cond(boolector_cond(this->handle, 
                                        t_uge, 
                                        static_cast<BoolectorNode*>(A.get()), 
                                        static_cast<BoolectorNode*>(B.get())));
    std::shared_ptr<void> res(boolector_cond(this->handle, 
                                        t_eq, 
                                        static_cast<BoolectorNode*>(this->prover_zero.get()), 
                                        static_cast<BoolectorNode*>(t_cond.get())));
    boolector_release(this->handle, t_and);
    boolector_release(this->handle, t_eq);
    boolector_release(this->handle, t_uge);
    this->exprs.insert(t_cond);
    this->exprs.insert(res);
    return res;
}

std::shared_ptr<void> BoolectorProver::prover_mk_not(std::shared_ptr<void> const &A)
{
    BoolectorNode *t_ult = boolector_ult(this->handle, static_cast<BoolectorNode*>(A.get()), static_cast<BoolectorNode*>(this->prover_x.get()));
    BoolectorNode *t_xor = boolector_xor(this->handle, static_cast<BoolectorNode*>(A.get()), static_cast<BoolectorNode*>(this->prover_one.get()));
    std::shared_ptr<void> res(boolector_cond(this->handle, 
                                        t_ult, 
                                        t_xor, 
                                        static_cast<BoolectorNode*>(this->prover_x.get())));
    boolector_release(this->handle, t_ult);
    boolector_release(this->handle, t_xor);
    this->exprs.insert(res);
    return res;
}

std::shared_ptr<void> BoolectorProver::prover_mk_DC(std::shared_ptr<void> const &C, std::shared_ptr<void> const &D)
{
    BoolectorNode *t_uge = boolector_ugt(this->handle, static_cast<BoolectorNode*>(D.get()), static_cast<BoolectorNode*>(this->prover_one.get()));
    std::shared_ptr<void> res(boolector_cond(this->handle, 
                                        t_uge, 
                                        static_cast<BoolectorNode*>(this->prover_x.get()), 
                                        static_cast<BoolectorNode*>(C.get())));
    boolector_release(this->handle, t_uge);
    this->exprs.insert(res);
    return res;
}

std::shared_ptr<void> BoolectorProver::prover_mk_HMUX(std::shared_ptr<void> const &I0, std::shared_ptr<void> const &I1, std::shared_ptr<void> const &S)
{
    BoolectorNode *t_ult = boolector_ult(this->handle, static_cast<BoolectorNode*>(S.get()), static_cast<BoolectorNode*>(this->prover_x.get()));
    BoolectorNode *t_ugt = boolector_ugt(this->handle, static_cast<BoolectorNode*>(S.get()), static_cast<BoolectorNode*>(this->prover_one.get()));
    std::shared_ptr<void> t_cond_gt(boolector_cond(this->handle, 
                                                t_ugt, 
                                                static_cast<BoolectorNode*>(I1.get()), 
                                                static_cast<BoolectorNode*>(I0.get())));
    BoolectorNode *t_eq = boolector_eq(this->handle, static_cast<BoolectorNode*>(I0.get()), static_cast<BoolectorNode*>(I1.get()));
    std::shared_ptr<void> t_cond_eq(boolector_cond(this->handle, 
                                                t_eq, 
                                                static_cast<BoolectorNode*>(I0.get()), 
                                                static_cast<BoolectorNode*>(this->prover_x.get())));
    std::shared_ptr<void> res(boolector_cond(this->handle, 
                                            t_ult, 
                                            static_cast<BoolectorNode*>(t_cond_gt.get()), 
                                            static_cast<BoolectorNode*>(t_cond_eq.get())));
    boolector_release(this->handle, t_ult);
    boolector_release(this->handle, t_ugt);
    boolector_release(this->handle, t_eq);
    this->exprs.insert(t_cond_gt);
    this->exprs.insert(t_cond_eq);
    this->exprs.insert(res);
    return res;
}

std::shared_ptr<void> BoolectorProver::prover_mk_exor(std::shared_ptr<void> const &A, std::shared_ptr<void> const &B)
{
    std::shared_ptr<void> t_eq_A_x(boolector_eq(this->handle, static_cast<BoolectorNode*>(A.get()), static_cast<BoolectorNode*>(this->prover_x.get())));
    std::shared_ptr<void> t_eq_A_B(boolector_eq(this->handle, static_cast<BoolectorNode*>(A.get()), static_cast<BoolectorNode*>(B.get())));
    std::shared_ptr<void> res(boolector_or(this->handle, static_cast<BoolectorNode*>(t_eq_A_x.get()), static_cast<BoolectorNode*>(t_eq_A_B.get())));
    this->exprs.insert(t_eq_A_x);
    this->exprs.insert(t_eq_A_B);
    this->exprs.insert(res);
    return res;
}

std::shared_ptr<void> BoolectorProver::prover_mk_and_exor(std::vector<std::shared_ptr<void> > &exors)
{
    if (exors.empty())
    {
        return NULL;
    }
    std::shared_ptr<void> res = exors[0];
    std::vector<std::shared_ptr<void> >::iterator it_ = exors.begin() + 1;
    std::vector<std::shared_ptr<void> >::iterator it_end = exors.end();
    while (it_ != it_end)
    {
        res = std::shared_ptr<void>(boolector_and(this->handle, static_cast<BoolectorNode*>(res.get()), static_cast<BoolectorNode*>((it_++)->get())));
        this->exprs.insert(res);
    }
    return res;
}

void BoolectorProver::handleQuery(std::shared_ptr<void> const &queryExpr, uint32_t timeout, FILE *fout)
{
    BoolectorNode *formula = boolector_not(this->handle, static_cast<BoolectorNode*>(queryExpr.get()));
    boolector_assert(this->handle, formula);
    int result = boolector_limited_sat(this->handle, -1, -1);
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
            const char *assign = boolector_bv_assignment(this->handle, static_cast<BoolectorNode*>(pi.get()));
            const char *symbol = boolector_get_symbol(this->handle, static_cast<BoolectorNode*>(pi.get()));
            fprintf(fout, "%s %s\n", symbol, assign);
            boolector_free_bv_assignment(this->handle, assign);
        }
        break;
    default:
        printf("Unhandled error\n");
    }
    boolector_release(this->handle, formula);
}

void BoolectorProver::handleQuery_EQ(std::shared_ptr<void> const &left, std::shared_ptr<void> const &right, uint32_t timeout, FILE *fout)
{
    std::shared_ptr<void> t_eq(boolector_eq(this->handle, static_cast<BoolectorNode*>(left.get()), static_cast<BoolectorNode*>(right.get())));
    this->handleQuery(t_eq, timeout, fout);
    boolector_release(this->handle, static_cast<BoolectorNode*>(t_eq.get()));
}

void BoolectorProver::handleQuery_Impl(std::shared_ptr<void> const &left, std::shared_ptr<void> const &right, uint32_t timeout, FILE *fout)
{
    std::shared_ptr<void> t_im(boolector_implies(this->handle, static_cast<BoolectorNode*>(left.get()), static_cast<BoolectorNode*>(right.get())));
    this->handleQuery(t_im, timeout, fout);
    boolector_release(this->handle, static_cast<BoolectorNode*>(t_im.get()));
}

void BoolectorProver::handleQuery_Impl(std::shared_ptr<void> const &right, uint32_t timeout, FILE *fout)
{
    std::shared_ptr<void> t_im(boolector_implies(this->handle, static_cast<BoolectorNode*>(prover_mk_and_exor(this->assert_exprs).get()), static_cast<BoolectorNode*>(right.get())));
    this->handleQuery(t_im, timeout, fout);
    boolector_release(this->handle, static_cast<BoolectorNode*>(t_im.get()));
}

/***************** test every operators **********************/
void BoolectorProver::test()
{
    boolector_set_opt(this->handle, BTOR_OPT_INCREMENTAL, 1);
    Prover::test();
}

void BoolectorProver::test_NAND() {}
void BoolectorProver::test_NOR() {}
void BoolectorProver::test_XNOR() {}

void BoolectorProver::test_EXOR()
{
    this->handleQuery(prover_mk_exor(this->prover_zero, this->prover_zero), 10, stdout);
    this->handleQuery(std::shared_ptr<void>(boolector_not(this->handle, static_cast<BoolectorNode*>(prover_mk_exor(this->prover_zero, this->prover_one).get()))), 10, stdout);
    this->handleQuery(std::shared_ptr<void>(boolector_not(this->handle, static_cast<BoolectorNode*>(prover_mk_exor(this->prover_zero, this->prover_x).get()))), 10, stdout);
    this->handleQuery(std::shared_ptr<void>(boolector_not(this->handle, static_cast<BoolectorNode*>(prover_mk_exor(this->prover_one, this->prover_zero).get()))), 10, stdout);
    this->handleQuery(prover_mk_exor(this->prover_one, this->prover_one), 10, stdout);
    this->handleQuery(std::shared_ptr<void>(boolector_not(this->handle, static_cast<BoolectorNode*>(prover_mk_exor(this->prover_one, this->prover_x).get()))), 10, stdout);
    this->handleQuery(prover_mk_exor(this->prover_x, this->prover_zero), 10, stdout);
    this->handleQuery(prover_mk_exor(this->prover_x, this->prover_one), 10, stdout);
    this->handleQuery(prover_mk_exor(this->prover_x, this->prover_x), 10, stdout);
}