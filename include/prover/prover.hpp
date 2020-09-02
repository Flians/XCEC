#ifndef _PROVER_H_
#define _PROVER_H_

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>
#include <unordered_set>

class Prover
{
protected:
    std::unordered_set<std::shared_ptr<void>> exprs;
    std::vector<std::shared_ptr<void>> assert_exprs;
    std::vector<std::shared_ptr<void>> in_exprs;

public:
    std::shared_ptr<void> prover_zero;
    std::shared_ptr<void> prover_one;
    std::shared_ptr<void> prover_x;
    std::shared_ptr<void> prover_undefined;

    Prover()
    {
        exprs.clear();
        std::vector<std::shared_ptr<void> >().swap(assert_exprs);
        std::vector<std::shared_ptr<void> >().swap(in_exprs);
    }

    virtual ~Prover()
    {
        this->exprs.clear();
        std::vector<std::shared_ptr<void> >().swap(this->assert_exprs);
        std::vector<std::shared_ptr<void> >().swap(this->in_exprs);
        printf("The Prover is destroyed!\n");
    }

    virtual std::shared_ptr<void> prover_mk_variable(const std::string &name) = 0;

    virtual std::shared_ptr<void> prover_mk_and(std::shared_ptr<void> const &, std::shared_ptr<void> const &) = 0;

    virtual std::shared_ptr<void> prover_mk_and(std::vector<std::shared_ptr<void> > &exprs)
    {
        if (exprs.empty())
        {
            return NULL;
        }
        std::shared_ptr<void> res = exprs[0];
        std::vector<std::shared_ptr<void> >::iterator it_ = exprs.begin() + 1;
        std::vector<std::shared_ptr<void> >::iterator it_end = exprs.end();
        while (it_ != it_end)
        {
            res = prover_mk_and(res, *(it_++));
            this->exprs.insert(res);
        }
        return res;
    }

    virtual std::shared_ptr<void> prover_mk_or(std::shared_ptr<void> const &, std::shared_ptr<void> const &) = 0;
    virtual std::shared_ptr<void> prover_mk_or(std::vector<std::shared_ptr<void> > &exprs)
    {
        if (exprs.empty())
        {
            return NULL;
        }
        std::shared_ptr<void> res = exprs[0];
        std::vector<std::shared_ptr<void> >::iterator it_ = exprs.begin() + 1;
        std::vector<std::shared_ptr<void> >::iterator it_end = exprs.end();
        while (it_ != it_end)
        {
            res = prover_mk_or(res, *(it_++));
            this->exprs.insert(res);
        }
        return res;
    }

    virtual std::shared_ptr<void> prover_mk_xor(std::shared_ptr<void> const &, std::shared_ptr<void> const &) = 0;
    virtual std::shared_ptr<void> prover_mk_xor(std::vector<std::shared_ptr<void> > &exprs)
    {
        if (exprs.empty())
        {
            return NULL;
        }
        std::shared_ptr<void> res = exprs[0];
        std::vector<std::shared_ptr<void> >::iterator it_ = exprs.begin() + 1;
        std::vector<std::shared_ptr<void> >::iterator it_end = exprs.end();
        while (it_ != it_end)
        {
            res = prover_mk_xor(res, *(it_++));
            this->exprs.insert(res);
        }
        return res;
    }

    virtual std::shared_ptr<void> prover_mk_not(std::shared_ptr<void> const &) = 0;
    virtual std::shared_ptr<void> prover_mk_DC(std::shared_ptr<void> const &C, std::shared_ptr<void> const &D) = 0;
    virtual std::shared_ptr<void> prover_mk_HMUX(std::shared_ptr<void> const &I0, std::shared_ptr<void> const &I1, std::shared_ptr<void> const &s) = 0;
    virtual std::shared_ptr<void> prover_mk_exor(std::shared_ptr<void> const &, std::shared_ptr<void> const &) = 0;
    virtual std::shared_ptr<void> prover_mk_and_exor(std::vector<std::shared_ptr<void> > &exprs) = 0;

    virtual void handleQuery(std::shared_ptr<void> const &queryExpr, uint32_t timeout, FILE *fout) = 0;
    virtual void handleQuery_EQ(std::shared_ptr<void> const &left, std::shared_ptr<void> const &right, uint32_t timeout, FILE *fout) = 0;
    virtual void handleQuery_Impl(std::shared_ptr<void> const &left, std::shared_ptr<void> const &right, uint32_t timeout, FILE *fout) = 0;
    virtual void handleQuery_Impl(std::shared_ptr<void> const &right, uint32_t timeout, FILE *fout) = 0;

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
        this->handleQuery_EQ(prover_mk_and(this->prover_zero, this->prover_zero), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_zero, this->prover_one), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_zero, this->prover_x), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_one, this->prover_zero), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_one, this->prover_one), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_one, this->prover_x), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_x, this->prover_zero), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_x, this->prover_one), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_and(this->prover_x, this->prover_x), this->prover_x, 10, stdout);
    }

    void test_OR()
    {
        this->handleQuery_EQ(prover_mk_or(this->prover_zero, this->prover_zero), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_zero, this->prover_one), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_zero, this->prover_x), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_one, this->prover_zero), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_one, this->prover_one), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_one, this->prover_x), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_x, this->prover_zero), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_x, this->prover_one), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_or(this->prover_x, this->prover_x), this->prover_x, 10, stdout);
    }

    void test_XOR()
    {
        this->handleQuery_EQ(prover_mk_xor(this->prover_zero, this->prover_zero), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_zero, this->prover_one), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_zero, this->prover_x), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_one, this->prover_zero), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_one, this->prover_one), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_one, this->prover_x), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_x, this->prover_zero), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_x, this->prover_one), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_xor(this->prover_x, this->prover_x), this->prover_x, 10, stdout);
    }

    void test_INV()
    {
        this->handleQuery_EQ(prover_mk_not(this->prover_zero), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_not(this->prover_one), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_not(this->prover_x), this->prover_x, 10, stdout);
    }

    void test_DC()
    {
        this->handleQuery_EQ(prover_mk_DC(this->prover_zero, this->prover_zero), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_zero, this->prover_one), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_zero, this->prover_x), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_one, this->prover_zero), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_one, this->prover_one), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_one, this->prover_x), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_x, this->prover_zero), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_x, this->prover_one), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_DC(this->prover_x, this->prover_x), this->prover_x, 10, stdout);
    }

    void test_HMUX()
    {
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_zero, this->prover_zero), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_one, this->prover_zero), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_x, this->prover_zero), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_zero, this->prover_zero), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_one, this->prover_zero), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_x, this->prover_zero), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_zero, this->prover_zero), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_one, this->prover_zero), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_x, this->prover_zero), this->prover_x, 10, stdout);

        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_zero, this->prover_one), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_one, this->prover_one), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_x, this->prover_one), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_zero, this->prover_one), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_one, this->prover_one), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_x, this->prover_one), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_zero, this->prover_one), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_one, this->prover_one), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_x, this->prover_one), this->prover_x, 10, stdout);

        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_zero, this->prover_x), this->prover_zero, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_one, this->prover_x), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_zero, this->prover_x, this->prover_x), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_zero, this->prover_x), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_one, this->prover_x), this->prover_one, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_one, this->prover_x, this->prover_x), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_zero, this->prover_x), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_one, this->prover_x), this->prover_x, 10, stdout);
        this->handleQuery_EQ(prover_mk_HMUX(this->prover_x, this->prover_x, this->prover_x), this->prover_x, 10, stdout);
    }

    virtual void test_EXOR() = 0;
};

#endif