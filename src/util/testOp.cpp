#include "testOp.h"

testOp::testOp(/* args */)
{
}

testOp::~testOp()
{
}

void testOp::prove(Z3_ast f)
{
    Z3_solver s = Z3_mk_solver(logic);
    Z3_solver_inc_ref(logic, s);
    Z3_model m = 0;
    /* save the current state of the context */
    Z3_solver_push(logic, s);

    Z3_ast not_f = Z3_mk_not(logic, f);
    Z3_solver_assert(logic, s, not_f);

    printf("term: %s\n", Z3_ast_to_string(logic, not_f));

    switch (Z3_solver_check(logic, s)) {
    case Z3_L_FALSE:
        /* proved */
        printf("valid\n");
        break;
    case Z3_L_UNDEF:
        /* Z3 failed to prove/disprove f. */
        printf("unknown\n");
        m = Z3_solver_get_model(logic, s);
        if (m != 0) {
            Z3_model_inc_ref(logic, m);
            /* m should be viewed as a potential counterexample. */
            printf("potential counterexample:\n%s\n", Z3_model_to_string(logic, m));
        }
        break;
    case Z3_L_TRUE:
        /* disproved */
        printf("invalid\n");
        m = Z3_solver_get_model(logic, s);
        if (m) {
            Z3_model_inc_ref(logic, m);
            /* the model returned by Z3 is a counterexample */
            printf("counterexample:\n%s\n", Z3_model_to_string(logic, m));
        }
        break;
    }
    if (m) Z3_model_dec_ref(logic, m);

    /* restore scope */
    Z3_solver_pop(logic, s, 1);

    Z3_solver_dec_ref(logic, s);
}

void testOp::prove(Z3_ast left, Z3_ast right, int op)
{
    Z3_solver s = Z3_mk_solver(logic);
    Z3_solver_inc_ref(logic, s);

    Z3_model m = 0;
    /* save the current state of the context */
    Z3_solver_push(logic, s);

    Z3_ast not_f = Z3_mk_not(logic, Z3_mk_eq(logic, left, right));
    Z3_solver_assert(logic, s, not_f);

    switch (Z3_solver_check(logic, s)) {
    case Z3_L_FALSE:
        /* proved */
        printf("valid\n");
        break;
    case Z3_L_UNDEF:
        /* Z3 failed to prove/disprove f. */
        printf("unknown\n");
        m = Z3_solver_get_model(logic, s);
        if (m != 0) {
            Z3_model_inc_ref(logic, m);
            /* m should be viewed as a potential counterexample. */
            printf("potential counterexample:\n%s\n", Z3_model_to_string(logic, m));
        }
        break;
    case Z3_L_TRUE:
        /* disproved */
        printf("invalid\n");
        m = Z3_solver_get_model(logic, s);
        if (m) {
            Z3_model_inc_ref(logic, m);
            /* the model returned by Z3 is a counterexample */
            printf("counterexample:\n%s\n", Z3_model_to_string(logic, m));
        }
        break;
    }
    if (m) Z3_model_dec_ref(logic, m);

    /* restore scope */
    Z3_solver_pop(logic, s, 1);

    Z3_solver_dec_ref(logic, s);
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
    prove(z3_mk_or(z3_zero, z3_zero), z3_zero);
    prove(z3_mk_or(z3_zero, z3_one), z3_one);
    prove(z3_mk_or(z3_zero, z3_x), z3_x);
    prove(z3_mk_or(z3_one, z3_zero), z3_one);
    prove(z3_mk_or(z3_one, z3_one), z3_one);
    prove(z3_mk_or(z3_one, z3_x), z3_one);
    prove(z3_mk_or(z3_x, z3_zero), z3_x);
    prove(z3_mk_or(z3_x, z3_one), z3_one);
    prove(z3_mk_or(z3_x, z3_x), z3_x);
}
void testOp::test_NOR() {}
void testOp::test_XOR()
{
    prove(z3_mk_xor(z3_zero, z3_zero), z3_zero);
    prove(z3_mk_xor(z3_zero, z3_one), z3_one);
    prove(z3_mk_xor(z3_zero, z3_x), z3_x);
    prove(z3_mk_xor(z3_one, z3_zero), z3_one);
    prove(z3_mk_xor(z3_one, z3_one), z3_zero);
    prove(z3_mk_xor(z3_one, z3_x), z3_x);
    prove(z3_mk_xor(z3_x, z3_zero), z3_x);
    prove(z3_mk_xor(z3_x, z3_one), z3_x);
    prove(z3_mk_xor(z3_x, z3_x), z3_x);
}
void testOp::test_XNOR() {}
void testOp::test_INV()
{
    prove(z3_mk_not(z3_zero), z3_one);
    prove(z3_mk_not(z3_one), z3_zero);
    prove(z3_mk_not(z3_x), z3_x);
}
void testOp::test_DC()
{
    prove(z3_mk_DC(z3_zero, z3_zero), z3_zero);
    prove(z3_mk_DC(z3_zero, z3_one), z3_x);
    prove(z3_mk_DC(z3_zero, z3_x), z3_x);
    prove(z3_mk_DC(z3_one, z3_zero), z3_one);
    prove(z3_mk_DC(z3_one, z3_one), z3_x);
    prove(z3_mk_DC(z3_one, z3_x), z3_x);
    prove(z3_mk_DC(z3_x, z3_zero), z3_x);
    prove(z3_mk_DC(z3_x, z3_one), z3_x);
    prove(z3_mk_DC(z3_x, z3_x), z3_x);
}
void testOp::test_HMUX()
{
    prove(z3_mk_HMUX(z3_zero, z3_zero, z3_zero), z3_zero);
    prove(z3_mk_HMUX(z3_zero, z3_zero, z3_one), z3_zero);
    prove(z3_mk_HMUX(z3_zero, z3_zero, z3_x), z3_zero);
    prove(z3_mk_HMUX(z3_zero, z3_one, z3_zero), z3_one);
    prove(z3_mk_HMUX(z3_zero, z3_one, z3_one), z3_one);
    prove(z3_mk_HMUX(z3_zero, z3_one, z3_x), z3_one);
    prove(z3_mk_HMUX(z3_zero, z3_x, z3_zero), z3_x);
    prove(z3_mk_HMUX(z3_zero, z3_x, z3_one), z3_x);
    prove(z3_mk_HMUX(z3_zero, z3_x, z3_x), z3_x);

    prove(z3_mk_HMUX(z3_one, z3_zero, z3_zero), z3_zero);
    prove(z3_mk_HMUX(z3_one, z3_zero, z3_one), z3_one);
    prove(z3_mk_HMUX(z3_one, z3_zero, z3_x), z3_x);
    prove(z3_mk_HMUX(z3_one, z3_one, z3_zero), z3_zero);
    prove(z3_mk_HMUX(z3_one, z3_one, z3_one), z3_one);
    prove(z3_mk_HMUX(z3_one, z3_one, z3_x), z3_x);
    prove(z3_mk_HMUX(z3_one, z3_x, z3_zero), z3_zero);
    prove(z3_mk_HMUX(z3_one, z3_x, z3_one), z3_one);
    prove(z3_mk_HMUX(z3_one, z3_x, z3_x), z3_x);

    prove(z3_mk_HMUX(z3_x, z3_zero, z3_zero), z3_zero);
    prove(z3_mk_HMUX(z3_x, z3_zero, z3_one), z3_x);
    prove(z3_mk_HMUX(z3_x, z3_zero, z3_x), z3_x);
    prove(z3_mk_HMUX(z3_x, z3_one, z3_zero), z3_x);
    prove(z3_mk_HMUX(z3_x, z3_one, z3_one), z3_one);
    prove(z3_mk_HMUX(z3_x, z3_one, z3_x), z3_x);
    prove(z3_mk_HMUX(z3_x, z3_x, z3_zero), z3_x);
    prove(z3_mk_HMUX(z3_x, z3_x, z3_one), z3_x);
    prove(z3_mk_HMUX(z3_x, z3_x, z3_x), z3_x);
}
void testOp::test_EXOR()
{
    Z3_ast args[] = {z3_mk_exor(z3_zero, z3_one)};
    prove(Z3_mk_or(logic, 1, args));

    prove(Z3_mk_not(logic, z3_mk_exor(z3_zero, z3_zero)));
    prove(z3_mk_exor(z3_zero, z3_one));
    prove(z3_mk_exor(z3_zero, z3_x));
    prove(z3_mk_exor(z3_one, z3_zero));
    prove(Z3_mk_not(logic, z3_mk_exor(z3_one, z3_one)));
    prove(z3_mk_exor(z3_one, z3_x));
    prove(Z3_mk_not(logic, z3_mk_exor(z3_x, z3_zero)));
    prove(Z3_mk_not(logic, z3_mk_exor(z3_x, z3_one)));
    prove(Z3_mk_not(logic, z3_mk_exor(z3_x, z3_x)));
}
