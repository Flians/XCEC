#ifndef _Z3PROVER_H_
#define _Z3PROVER_H_

#include <vector>
#include <z3/z3++.h>

using namespace std;

class Z3Prover
{
private:
   /* data */
public:
   Z3_context logic;
   Z3_sort bv_sort;

   Z3_ast z3_zero;
   Z3_ast z3_one;
   Z3_ast z3_x;
   Z3_ast z3_undefined;

   Z3Prover();
   /**
    * init z3
    * 
    * @param timeout millisecond
    */
   Z3Prover(unsigned timeout);
   ~Z3Prover();

   Z3_context &get_context();
   // C API of Z3
   /**
   \brief Simpler error handler.
 */
   static void error_handler(Z3_context c, Z3_error_code e);
   /**
   \brief exit if unreachable code was reached.
*/
   void unreachable();

   Z3_ast z3_mk_variable(string &name, Z3_solver &z3_sol);

   Z3_ast z3_mk_and(const Z3_ast &, const Z3_ast &);
   Z3_ast z3_mk_and(vector<Z3_ast> &);
   Z3_ast z3_mk_or(const Z3_ast &, const Z3_ast &);
   Z3_ast z3_mk_or(vector<Z3_ast> &);
   Z3_ast z3_mk_xor(const Z3_ast &, const Z3_ast &);
   Z3_ast z3_mk_xor(vector<Z3_ast> &);
   Z3_ast z3_mk_not(const Z3_ast &);
   Z3_ast z3_mk_DC(const Z3_ast &C, const Z3_ast &D);
   Z3_ast z3_mk_HMUX(const Z3_ast &I0, const Z3_ast &I1, const Z3_ast &S);
   Z3_ast z3_mk_exor(const Z3_ast &, const Z3_ast &);

   /****** print the information ******/
   /**
   \brief Display a symbol in the given output stream.
*/
   void display_symbol(Z3_context &c, FILE *out, Z3_symbol s);
   /**
   \brief Display the given type.
*/
   void display_sort(Z3_context &, FILE *out, Z3_sort ty);
   /**
   \brief Custom ast pretty printer.

   This function demonstrates how to use the API to navigate terms.
*/
   void display_ast(Z3_context &c, FILE *out, Z3_ast v);
   /**
   \brief Custom function interpretations pretty printer.
*/
   void display_function_interpretations(Z3_context &c, FILE *out, Z3_model m);
   /**
   \brief Custom model pretty printer.
*/
   void display_model(Z3_context &c, FILE *out, Z3_model m);
   /**
   \brief Check whether the logical context is satisfiable, and compare the result with the expected result.
   If the context is satisfiable, then display the model.
*/
   void check(Z3_context &ctx, Z3_solver &s, Z3_lbool expected_result, FILE *fout);
   void check_(Z3_context &logic, Z3_solver &z3_sol, Z3_lbool expected_result, FILE *fout);

   /**
    * Configure Z3 backend for C++ API
    * 
    * @param priority: pareto, box, lex
    * @param timeout millisecond
    */
   z3::params config_z3(z3::context &logic, string &priority, unsigned timeout);

   /***************** test every operators **********************/
   void test();
   void test_AND();
   void test_NAND();
   void test_OR();
   void test_NOR();
   void test_XOR();
   void test_XNOR();
   void test_INV();
   void test_DC();
   void test_HMUX();
   void test_EXOR();
};

#endif