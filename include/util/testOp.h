#include "libhead.h"

class testOp
{
private:
    /* data */
public:
    testOp(/* args */);
    ~testOp();

    static void prove(z3::expr conjecture);
    static void prove(z3::expr left, z3::expr right, int op = 0);

    static void test();

    static void test_AND();
    static void test_NAND();
    static void test_OR();
    static void test_NOR();
    static void test_XOR();
    static void test_XNOR();
    static void test_INV();
    static void test_DC();
    static void test_HMUX();
    static void test_EXOR();
};
