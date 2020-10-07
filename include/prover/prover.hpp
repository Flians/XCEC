#ifndef _PROVER_H_
#define _PROVER_H_

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <assert.h>
#include <unordered_set>

class Prover
{
protected:
    std::unordered_set<void *> exprs;
    std::vector<void *> assert_exprs;
    std::vector<void *> in_exprs;

public:
    void *prover_zero;
    void *prover_one;
    void *prover_x;
    void *prover_undefined;

    Prover()
    {
        exprs.clear();
        std::vector<void *>().swap(assert_exprs);
        std::vector<void *>().swap(in_exprs);
    }

    virtual ~Prover()
    {
        this->exprs.clear();
        std::vector<void *>().swap(this->assert_exprs);
        std::vector<void *>().swap(this->in_exprs);
        printf("The Prover is destroyed!\n");
    }

    virtual void *prover_mk_variable(const std::string &name) = 0;

    virtual void *prover_mk_and(void *const &, void *const &) = 0;

    virtual void *prover_mk_and(std::vector<void *> &exprs)
    {
        if (exprs.empty())
        {
            return NULL;
        }
        void *res = exprs[0];
        std::vector<void *>::iterator it_ = exprs.begin() + 1;
        std::vector<void *>::iterator it_end = exprs.end();
        while (it_ != it_end)
        {
            res = prover_mk_and(res, *(it_++));
            this->exprs.insert(res);
        }
        return res;
    }

    virtual void *prover_mk_or(void *const &, void *const &) = 0;
    virtual void *prover_mk_or(std::vector<void *> &exprs)
    {
        if (exprs.empty())
        {
            return NULL;
        }
        void *res = exprs[0];
        std::vector<void *>::iterator it_ = exprs.begin() + 1;
        std::vector<void *>::iterator it_end = exprs.end();
        while (it_ != it_end)
        {
            res = prover_mk_or(res, *(it_++));
            this->exprs.insert(res);
        }
        return res;
    }

    virtual void *prover_mk_xor(void *const &, void *const &) = 0;
    virtual void *prover_mk_xor(std::vector<void *> &exprs)
    {
        if (exprs.empty())
        {
            return NULL;
        }
        void *res = exprs[0];
        std::vector<void *>::iterator it_ = exprs.begin() + 1;
        std::vector<void *>::iterator it_end = exprs.end();
        while (it_ != it_end)
        {
            res = prover_mk_xor(res, *(it_++));
            this->exprs.insert(res);
        }
        return res;
    }

    virtual void *prover_mk_not(void *const &) = 0;
    virtual void *prover_mk_DC(void *const &C, void *const &D) = 0;
    virtual void *prover_mk_HMUX(void *const &I0, void *const &I1, void *const &s) = 0;
    virtual void *prover_mk_exor(void *const &, void *const &) = 0;
    virtual void *prover_mk_and_exor(std::vector<void *> &exprs) = 0;

    virtual void handleQuery(void *const &queryExpr, int timeout, int max_conflicts, FILE *fout) = 0;
    virtual void handleQuery_EQ(void *const &left, void *const &right, int timeout, int max_conflicts, FILE *fout) = 0;
    virtual void handleQuery_Impl(void *const &left, void *const &right, int timeout, int max_conflicts, FILE *fout) = 0;
    virtual void handleQuery_Impl(void *const &right, int timeout, int max_conflicts, FILE *fout) = 0;
    virtual void handleQuery_incremental(std::vector<void *> &exors, int timeout, int max_conflicts, FILE *fout) = 0;

    /***************** test every operators **********************/
    virtual void test()
    {
        printf("test AND\n");
        test_AND();
        printf("test OR\n");
        test_OR();
        printf("test XOR\n");
        test_XOR();
        printf("test INV\n");
        test_INV();
        printf("test DC\n");
        test_DC();
        printf("test HMUX\n");
        test_HMUX();
        printf("test EXOR\n");
        test_EXOR();
    }

    void test_AND()
    {
        this->handleQuery_EQ(prover_mk_and(this->prover_zero, this->prover_zero), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_zero, this->prover_one), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_zero, this->prover_x), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_one, this->prover_zero), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_one, this->prover_one), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_one, this->prover_x), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_x, this->prover_zero), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_x, this->prover_one), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_x, this->prover_x), this->prover_x, 1000, 10000, stdout);
    }

    void test_OR()
    {
        this->handleQuery_EQ(prover_mk_or(this->prover_zero, this->prover_zero), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_zero, this->prover_one), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_zero, this->prover_x), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_one, this->prover_zero), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_one, this->prover_one), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_one, this->prover_x), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_x, this->prover_zero), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_x, this->prover_one), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_x, this->prover_x), this->prover_x, 1000, 10000, stdout);
    }

    void test_XOR()
    {
        this->handleQuery_EQ(prover_mk_xor(this->prover_zero, this->prover_zero), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_zero, this->prover_one), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_zero, this->prover_x), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_one, this->prover_zero), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_one, this->prover_one), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_one, this->prover_x), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_x, this->prover_zero), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_x, this->prover_one), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_x, this->prover_x), this->prover_x, 1000, 10000, stdout);
    }

    void test_INV()
    {
        this->handleQuery_EQ(prover_mk_not(this->prover_zero), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_not(this->prover_one), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_not(this->prover_x), this->prover_x, 1000, 10000, stdout);
    }

    void test_DC()
    {
        this->handleQuery_EQ(prover_mk_DC(this->prover_zero, this->prover_zero), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_zero, this->prover_one), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_zero, this->prover_x), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_one, this->prover_zero), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_one, this->prover_one), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_one, this->prover_x), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_x, this->prover_zero), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_x, this->prover_one), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_x, this->prover_x), this->prover_x, 1000, 10000, stdout);
    }

    void test_HMUX()
    {
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_zero, this->prover_zero), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_one, this->prover_zero), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_x, this->prover_zero), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_zero, this->prover_zero), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_one, this->prover_zero), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_x, this->prover_zero), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_zero, this->prover_zero), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_one, this->prover_zero), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_x, this->prover_zero), this->prover_x, 1000, 10000, stdout);

        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_zero, this->prover_one), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_one, this->prover_one), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_x, this->prover_one), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_zero, this->prover_one), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_one, this->prover_one), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_x, this->prover_one), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_zero, this->prover_one), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_one, this->prover_one), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_x, this->prover_one), this->prover_x, 1000, 10000, stdout);

        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_zero, this->prover_x), this->prover_zero, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_one, this->prover_x), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_x, this->prover_x), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_zero, this->prover_x), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_one, this->prover_x), this->prover_one, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_x, this->prover_x), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_zero, this->prover_x), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_one, this->prover_x), this->prover_x, 1000, 10000, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_x, this->prover_x), this->prover_x, 1000, 10000, stdout);
    }

    virtual void test_EXOR() = 0;
};

#endif