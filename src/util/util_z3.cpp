#include "util_z3.h"

/**
   \brief Simpler error handler.
 */
inline void error_handler(Z3_context c, Z3_error_code e)
{
    cerr << "Error code: " << e << "\nBUG: incorrect use of Z3." << endl;
    exit(1);
}

/**
   \brief exit if unreachable code was reached.
*/
void unreachable()
{
    cerr << "BUG: unreachable code was reached." << endl;
    exit(1);
}

Z3_context mk_context(unsigned timeout)
{
    Z3_config cfg;
    cfg = Z3_mk_config();
    Z3_set_param_value(cfg, "model", "true");
    Z3_set_param_value(cfg, "proof", "true");
    Z3_set_param_value(cfg, "timeout", to_string(timeout).c_str());

    Z3_context ctx;
    ctx = Z3_mk_context(cfg);
    Z3_set_error_handler(ctx, error_handler);

    Z3_del_config(cfg);
    return ctx;
}

Z3_context logic;
Z3_sort bv_sort;

Z3_ast z3_zero;
Z3_ast z3_one;
Z3_ast z3_x;
Z3_ast z3_undefined;

void init_z3(unsigned timeout)
{
    logic = mk_context(timeout);
    bv_sort = Z3_mk_bv_sort(logic, 2);
    z3_zero = Z3_mk_unsigned_int(logic, 0, bv_sort);
    z3_one = Z3_mk_unsigned_int(logic, 1, bv_sort);
    z3_x = Z3_mk_unsigned_int(logic, 2, bv_sort);
    z3_undefined = Z3_mk_unsigned_int(logic, 3, bv_sort);
}

Z3_ast z3_mk_and(const Z3_ast &A, const Z3_ast &B)
{
    // return z3::ite(A == z3_zero || B == z3_zero, z3_zero, z3::max(A, B));
    return Z3_mk_ite(logic, Z3_mk_eq(logic, Z3_mk_bvxor(logic, A, B), z3_undefined), z3_x, Z3_mk_bvand(logic, A, B));
}

Z3_ast z3_mk_and(vector<Z3_ast> &exprs)
{
    Z3_ast res = exprs[0];
    vector<Z3_ast>::iterator it_ = exprs.begin();
    vector<Z3_ast>::iterator it_end = exprs.end() - 1;
    while (it_ != it_end)
    {
        res = z3_mk_and(res, *(++it_));
    }
    return res;
}

Z3_ast z3_mk_or(const Z3_ast &A, const Z3_ast &B)
{
    // return z3::ite(A == z3_one || B == z3_one, z3_one, z3::max(A, B));
    return Z3_mk_ite(logic, Z3_mk_eq(logic, Z3_mk_bvxor(logic, A, B), z3_undefined), z3_one, Z3_mk_bvor(logic, A, B));
}

Z3_ast z3_mk_or(vector<Z3_ast> &exprs)
{
    Z3_ast res = exprs[0];
    vector<Z3_ast>::iterator it_ = exprs.begin();
    vector<Z3_ast>::iterator it_end = exprs.end() - 1;
    while (it_ != it_end)
    {
        res = z3_mk_or(res, *(++it_));
    }
    return res;
}

Z3_ast z3_mk_xor(const Z3_ast &A, const Z3_ast &B)
{
    // return z3::ite(A == z3_one && B == z3_one, z3_zero, z3::max(A, B));
    return Z3_mk_ite(logic, Z3_mk_eq(logic, Z3_mk_bvand(logic, A, B), z3_one), z3_zero, Z3_mk_ite(logic, Z3_mk_bvuge(logic, A, B), A, B));
}

Z3_ast z3_mk_xor(vector<Z3_ast> &exprs)
{
    Z3_ast res = exprs[0];
    vector<Z3_ast>::iterator it_ = exprs.begin();
    vector<Z3_ast>::iterator it_end = exprs.end() - 1;
    while (it_ != it_end)
    {
        res = z3_mk_xor(res, *(++it_));
    }
    return res;
}

Z3_ast z3_mk_not(const Z3_ast &A)
{
    // return z3::ite(A == z3_zero, z3_one, z3::ite(A == z3_one, z3_zero, z3_x));
    // return z3::ite(A == z3_x, z3_x, A ^ z3_one);
    return Z3_mk_ite(logic, Z3_mk_eq(logic, A, z3_x), z3_x, Z3_mk_bvxor(logic, A, z3_one));
}

Z3_ast z3_mk_DC(const Z3_ast &C, const Z3_ast &D)
{
    // return z3::ite(D == z3_zero, C, z3_x);
    return Z3_mk_ite(logic, Z3_mk_eq(logic, D, z3_zero), C, z3_x);
}

Z3_ast z3_mk_HMUX(const Z3_ast &I0, const Z3_ast &I1, const Z3_ast &S)
{
    // return z3::ite(S == z3_x, z3::ite(I0 == I1, I0, z3_x), z3::ite(S == z3_zero, I0, I1));
    return Z3_mk_ite(logic, Z3_mk_eq(logic, S, z3_x),
                     Z3_mk_ite(logic, Z3_mk_eq(logic, I0, I1), I0, z3_x),
                     Z3_mk_ite(logic, Z3_mk_eq(logic, S, z3_zero), I0, I1));
}

Z3_ast z3_mk_exor(const Z3_ast &A, const Z3_ast &B)
{
    // return z3::ite(A == z3_x || A == B, z3_zero, z3_one);
    Z3_ast args[2] = {Z3_mk_eq(logic, A, z3_x), Z3_mk_eq(logic, A, B)};
    // return Z3_mk_ite(logic, Z3_mk_or(logic, 2, args), z3_zero, z3_one);
    return Z3_mk_or(logic, 2, args);
}

/****** print the information ******/

/**
   \brief Display a symbol in the given output stream.
*/
void display_symbol(Z3_context c, FILE *out, Z3_symbol s)
{
    switch (Z3_get_symbol_kind(c, s))
    {
    case Z3_INT_SYMBOL:
        fprintf(out, "#%d", Z3_get_symbol_int(c, s));
        break;
    case Z3_STRING_SYMBOL:
        fprintf(out, "%s", Z3_get_symbol_string(c, s));
        break;
    default:
        unreachable();
    }
}

/**
   \brief Display the given type.
*/
void display_sort(Z3_context c, FILE *out, Z3_sort ty)
{
    switch (Z3_get_sort_kind(c, ty))
    {
    case Z3_UNINTERPRETED_SORT:
        display_symbol(c, out, Z3_get_sort_name(c, ty));
        break;
    case Z3_BOOL_SORT:
        fprintf(out, "bool");
        break;
    case Z3_INT_SORT:
        fprintf(out, "int");
        break;
    case Z3_REAL_SORT:
        fprintf(out, "real");
        break;
    case Z3_BV_SORT:
        fprintf(out, "bv%d", Z3_get_bv_sort_size(c, ty));
        break;
    case Z3_ARRAY_SORT:
        fprintf(out, "[");
        display_sort(c, out, Z3_get_array_sort_domain(c, ty));
        fprintf(out, "->");
        display_sort(c, out, Z3_get_array_sort_range(c, ty));
        fprintf(out, "]");
        break;
    case Z3_DATATYPE_SORT:
        if (Z3_get_datatype_sort_num_constructors(c, ty) != 1)
        {
            fprintf(out, "%s", Z3_sort_to_string(c, ty));
            break;
        }
        {
            unsigned num_fields = Z3_get_tuple_sort_num_fields(c, ty);
            unsigned i;
            fprintf(out, "(");
            for (i = 0; i < num_fields; ++i)
            {
                Z3_func_decl field = Z3_get_tuple_sort_field_decl(c, ty, i);
                if (i > 0)
                {
                    fprintf(out, ", ");
                }
                display_sort(c, out, Z3_get_range(c, field));
            }
            fprintf(out, ")");
            break;
        }
    default:
        fprintf(out, "unknown[");
        display_symbol(c, out, Z3_get_sort_name(c, ty));
        fprintf(out, "]");
        break;
    }
}

/**
   \brief Custom ast pretty printer.

   This function demonstrates how to use the API to navigate terms.
*/
void display_ast(Z3_context c, FILE *out, Z3_ast v)
{
    switch (Z3_get_ast_kind(c, v))
    {
    case Z3_NUMERAL_AST:
    {
        Z3_sort t;
        fprintf(out, "%s", Z3_get_numeral_string(c, v));
        t = Z3_get_sort(c, v);
        /*
        fprintf(out, ":");
        display_sort(c, out, t);
        */
        break;
    }
    case Z3_APP_AST:
    {
        unsigned i;
        Z3_app app = Z3_to_app(c, v);
        unsigned num_fields = Z3_get_app_num_args(c, app);
        Z3_func_decl d = Z3_get_app_decl(c, app);
        fprintf(out, "%s", Z3_func_decl_to_string(c, d));
        if (num_fields > 0)
        {
            fprintf(out, "[");
            for (i = 0; i < num_fields; ++i)
            {
                if (i > 0)
                {
                    fprintf(out, ", ");
                }
                display_ast(c, out, Z3_get_app_arg(c, app, i));
            }
            fprintf(out, "]");
        }
        break;
    }
    case Z3_QUANTIFIER_AST:
    {
        fprintf(out, "quantifier");
        ;
    }
    default:
        fprintf(out, "#unknown");
    }
}

/**
   \brief Custom function interpretations pretty printer.
*/
void display_function_interpretations(Z3_context c, FILE *out, Z3_model m)
{
    unsigned num_functions, i;

    fprintf(out, "function interpretations:\n");

    num_functions = Z3_model_get_num_funcs(c, m);
    for (i = 0; i < num_functions; ++i)
    {
        Z3_func_decl fdecl;
        Z3_symbol name;
        Z3_ast func_else;
        unsigned num_entries = 0, j;
        Z3_func_interp_opt finterp;

        fdecl = Z3_model_get_func_decl(c, m, i);
        finterp = Z3_model_get_func_interp(c, m, fdecl);
        Z3_func_interp_inc_ref(c, finterp);
        name = Z3_get_decl_name(c, fdecl);
        display_symbol(c, out, name);
        fprintf(out, " = {");
        if (finterp)
            num_entries = Z3_func_interp_get_num_entries(c, finterp);
        for (j = 0; j < num_entries; ++j)
        {
            unsigned num_args, k;
            Z3_func_entry fentry = Z3_func_interp_get_entry(c, finterp, j);
            Z3_func_entry_inc_ref(c, fentry);
            if (j > 0)
            {
                fprintf(out, ", ");
            }
            num_args = Z3_func_entry_get_num_args(c, fentry);
            fprintf(out, "(");
            for (k = 0; k < num_args; ++k)
            {
                if (k > 0)
                {
                    fprintf(out, ", ");
                }
                display_ast(c, out, Z3_func_entry_get_arg(c, fentry, k));
            }
            fprintf(out, "|->");
            display_ast(c, out, Z3_func_entry_get_value(c, fentry));
            fprintf(out, ")");
            Z3_func_entry_dec_ref(c, fentry);
        }
        if (num_entries > 0)
        {
            fprintf(out, ", ");
        }
        fprintf(out, "(else|->");
        func_else = Z3_func_interp_get_else(c, finterp);
        display_ast(c, out, func_else);
        fprintf(out, ")}\n");
        Z3_func_interp_dec_ref(c, finterp);
    }
}

/**
   \brief Custom model pretty printer.
*/
void display_model(Z3_context c, FILE *out, Z3_model m)
{
    unsigned num_constants;
    unsigned i;

    if (!m)
        return;

    num_constants = Z3_model_get_num_consts(c, m);
    for (i = 0; i < num_constants; ++i)
    {
        Z3_symbol name;
        Z3_func_decl cnst = Z3_model_get_const_decl(c, m, i);
        Z3_ast a, v;
        bool ok;
        name = Z3_get_decl_name(c, cnst);
        display_symbol(c, out, name);
        fprintf(out, " ");
        a = Z3_mk_app(c, cnst, 0, 0);
        v = a;
        ok = Z3_model_eval(c, m, a, 1, &v);
        (void)ok;
        display_ast(c, out, v);
        fprintf(out, "\n");
    }
}

void check(Z3_context logic, Z3_solver z3_sol, Z3_lbool expected_result, FILE *fout)
{
    Z3_model m = 0;
    Z3_lbool result = Z3_solver_check(logic, z3_sol);
    switch (result)
    {
    case Z3_L_UNDEF:
        /* Z3 failed to prove/disprove f. */
        printf(">>> unknown <<<\n");
    case Z3_L_FALSE:
        fprintf(fout, "EQ\n");
        // fout << "EQ" << endl;
        break;
    case Z3_L_TRUE:
        /* disproved */
        fprintf(fout, "NEQ\n");
        // fout << "NEQ" << endl;
        m = Z3_solver_get_model(logic, z3_sol);
        if (m)
        {
            Z3_model_inc_ref(logic, m);
            display_model(logic, fout, m);
        }
        break;
    }
    if (result != expected_result)
    {
        printf(">>> unexpected result <<<\n");
    }
    if (m)
    {
        Z3_model_dec_ref(logic, m);
    }
}

void check_(Z3_context logic, Z3_solver z3_sol, Z3_lbool expected_result, FILE *fout)
{
    Z3_model m = 0;
    Z3_lbool result = Z3_solver_check(logic, z3_sol);
    switch (result)
    {
    case Z3_L_FALSE:
        fprintf(fout, "NEQ\n");
        m = Z3_solver_get_model(logic, z3_sol);
        if (m)
        {
            Z3_model_inc_ref(logic, m);
            display_model(logic, fout, m);
        }
        break;
    case Z3_L_UNDEF:
        /* Z3 failed to prove/disprove f. */
        printf(">>> unknown <<<\n");
    case Z3_L_TRUE:
        fprintf(fout, "EQ\n");
        break;
    }
    if (result != expected_result)
    {
        printf(">>> unexpected result <<<\n");
    }
    if (m)
    {
        Z3_model_dec_ref(logic, m);
    }
}

/**
 * Configure Z3 backend for C++ API
 * 
 * @param priority: pareto, box, lex
 * @param timeout millisecond
 */
z3::params config_z3(z3::context logic, string priority, unsigned timeout)
{
    z3::set_param("timeout", (int)timeout);
    z3::params z3_param(logic);
    // http://smtlib.cs.uiowa.edu/logics-all.shtml
    logic.set("logic", "QF_BV");

    //z3_param.set(":opt.solution_prefix", "intermediate_result");
    //z3_param.set(":opt.dump_models", true);
    // z3_param.set(":opt.pb.compile_equality", true);
    // z3_param.set(":opt.priority", priority.c_str());
    z3_param.set("timeout", static_cast<unsigned>(timeout));

    return z3_param;
}