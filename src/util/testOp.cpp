#include "testOp.h"

testOp::testOp(/* args */)
{
}

testOp::~testOp()
{
}

void testOp::prove(z3::expr conjecture)
{
    z3::context &c = conjecture.ctx();
    z3::solver s(c);
    s.add(!conjecture);
    // std::cout << "conjecture:\n" << conjecture << "\n";
    if (s.check() == z3::unsat)
    {
        std::cout << "proved" << std::endl;
    }
    else
    {
        std::cout << "failed to prove" << std::endl;
        std::cout << "counterexample:\n"
                  << s.get_model() << std::endl;
    }
}

void testOp::prove(z3::expr left, z3::expr right, int op)
{
    z3::context &c = left.ctx();
    z3::solver s(c);
    z3::expr conjecture(c);
    switch (op)
    {
    case 0:
        conjecture = (left == right);
        break;

    default:
        conjecture = (left == right);
        break;
    }
    s.add(!conjecture);
    std::cout << "conjecture:\n"
              << conjecture << "\n";
    if (s.check() == z3::unsat)
    {
        std::cout << "proved" << std::endl;
    }
    else
    {
        std::cout << "failed to prove" << std::endl;
        std::cout << "counterexample:\n"
                  << s.get_model().eval(left) << std::endl;
    }
}

void testOp::test()
{
    std::cout << "test AND" << std::endl;
    test_AND();
    std::cout << "test NAND" << std::endl;
    test_NAND();
    std::cout << "test OR" << std::endl;
    test_OR();
    std::cout << "test NOR" << std::endl;
    test_NOR();
    std::cout << "test XOR" << std::endl;
    test_XOR();
    std::cout << "test XNOR" << std::endl;
    test_XNOR();
    std::cout << "test INV" << std::endl;
    test_INV();
    std::cout << "test DC" << std::endl;
    test_DC();
    std::cout << "test HMUX" << std::endl;
    test_HMUX();
    std::cout << "test EXOR" << std::endl;
    test_EXOR();
}

void testOp::test_AND()
{
    prove(z3_mk_and(z3_zero, z3_zero), z3_zero);
    prove(z3_mk_and(z3_zero, z3_one), z3_zero);
    prove(z3_mk_and(z3_zero, z3_x), z3_zero);
    prove(z3_mk_and(z3_one, z3_zero), z3_zero);
    prove(z3_mk_and(z3_one, z3_one), z3_one);
    prove(z3_mk_and(z3_one, z3_x), z3_x);
    prove(z3_mk_and(z3_x, z3_zero), z3_zero);
    prove(z3_mk_and(z3_x, z3_one), z3_x);
    prove(z3_mk_and(z3_x, z3_x), z3_x);
}
void testOp::test_NAND()
{
}
void testOp::test_OR()
{
    prove(z3_mk_or(z3_zero, z3_zero) == z3_zero);
    prove(z3_mk_or(z3_zero, z3_one) == z3_one);
    prove(z3_mk_or(z3_zero, z3_x) == z3_x);
    prove(z3_mk_or(z3_one, z3_zero) == z3_one);
    prove(z3_mk_or(z3_one, z3_one) == z3_one);
    prove(z3_mk_or(z3_one, z3_x) == z3_one);
    prove(z3_mk_or(z3_x, z3_zero) == z3_x);
    prove(z3_mk_or(z3_x, z3_one) == z3_one);
    prove(z3_mk_or(z3_x, z3_x) == z3_x);
}
void testOp::test_NOR() {}
void testOp::test_XOR()
{
    prove(z3_mk_xor(z3_zero, z3_zero) == z3_zero);
    prove(z3_mk_xor(z3_zero, z3_one) == z3_one);
    prove(z3_mk_xor(z3_zero, z3_x) == z3_x);
    prove(z3_mk_xor(z3_one, z3_zero) == z3_one);
    prove(z3_mk_xor(z3_one, z3_one) == z3_zero);
    prove(z3_mk_xor(z3_one, z3_x) == z3_x);
    prove(z3_mk_xor(z3_x, z3_zero) == z3_x);
    prove(z3_mk_xor(z3_x, z3_one) == z3_x);
    prove(z3_mk_xor(z3_x, z3_x) == z3_x);
}
void testOp::test_XNOR() {}
void testOp::test_INV()
{
    prove(z3_mk_not(z3_zero) == z3_one);
    prove(z3_mk_not(z3_one) == z3_zero);
    prove(z3_mk_not(z3_x) == z3_x);
}
void testOp::test_DC()
{
    prove(z3_mk_DC(z3_zero, z3_zero) == z3_zero);
    prove(z3_mk_DC(z3_zero, z3_one) == z3_zero);
    prove(z3_mk_DC(z3_zero, z3_x) == z3_x);
    prove(z3_mk_DC(z3_one, z3_zero) == z3_zero);
    prove(z3_mk_DC(z3_one, z3_one) == z3_one);
    prove(z3_mk_DC(z3_one, z3_x) == z3_x);
    prove(z3_mk_DC(z3_x, z3_zero) == z3_x);
    prove(z3_mk_DC(z3_x, z3_one) == z3_x);
    prove(z3_mk_DC(z3_x, z3_x) == z3_x);
}
void testOp::test_HMUX()
{
    prove(z3_mk_HMUX(z3_zero, z3_zero, z3_zero) == z3_zero);
    prove(z3_mk_HMUX(z3_zero, z3_zero, z3_one) == z3_zero);
    prove(z3_mk_HMUX(z3_zero, z3_zero, z3_x) == z3_zero);
    prove(z3_mk_HMUX(z3_zero, z3_one, z3_zero) == z3_one);
    prove(z3_mk_HMUX(z3_zero, z3_one, z3_one) == z3_one);
    prove(z3_mk_HMUX(z3_zero, z3_one, z3_x) == z3_one);
    prove(z3_mk_HMUX(z3_zero, z3_x, z3_zero) == z3_x);
    prove(z3_mk_HMUX(z3_zero, z3_x, z3_one) == z3_x);
    prove(z3_mk_HMUX(z3_zero, z3_x, z3_x) == z3_x);

    prove(z3_mk_HMUX(z3_one, z3_zero, z3_zero) == z3_zero);
    prove(z3_mk_HMUX(z3_one, z3_zero, z3_one) == z3_one);
    prove(z3_mk_HMUX(z3_one, z3_zero, z3_x) == z3_x);
    prove(z3_mk_HMUX(z3_one, z3_one, z3_zero) == z3_zero);
    prove(z3_mk_HMUX(z3_one, z3_one, z3_one) == z3_one);
    prove(z3_mk_HMUX(z3_one, z3_one, z3_x) == z3_x);
    prove(z3_mk_HMUX(z3_one, z3_x, z3_zero) == z3_zero);
    prove(z3_mk_HMUX(z3_one, z3_x, z3_one) == z3_one);
    prove(z3_mk_HMUX(z3_one, z3_x, z3_x) == z3_x);

    prove(z3_mk_HMUX(z3_x, z3_zero, z3_zero) == z3_zero);
    prove(z3_mk_HMUX(z3_x, z3_zero, z3_one) == z3_x);
    prove(z3_mk_HMUX(z3_x, z3_zero, z3_x) == z3_x);
    prove(z3_mk_HMUX(z3_x, z3_one, z3_zero) == z3_x);
    prove(z3_mk_HMUX(z3_x, z3_one, z3_one) == z3_one);
    prove(z3_mk_HMUX(z3_x, z3_one, z3_x) == z3_x);
    prove(z3_mk_HMUX(z3_x, z3_x, z3_zero) == z3_x);
    prove(z3_mk_HMUX(z3_x, z3_x, z3_one) == z3_x);
    prove(z3_mk_HMUX(z3_x, z3_x, z3_x) == z3_x);
}
void testOp::test_EXOR()
{
    prove(z3_mk_exor(z3_zero, z3_zero) == z3_zero);
    prove(z3_mk_exor(z3_zero, z3_one) == z3_one);
    prove(z3_mk_exor(z3_zero, z3_x) == z3_one);
    prove(z3_mk_exor(z3_one, z3_zero) == z3_one);
    prove(z3_mk_exor(z3_one, z3_one) == z3_zero);
    prove(z3_mk_exor(z3_one, z3_x) == z3_one);
    prove(z3_mk_exor(z3_x, z3_zero) == z3_zero);
    prove(z3_mk_exor(z3_x, z3_one) == z3_zero);
    prove(z3_mk_exor(z3_x, z3_x) == z3_zero);
}
