#include <tiramisu/tiramisu.h>

using namespace tiramisu;

int main(int argc, char **argv)
{
    tiramisu::init("function_seidel2d_LARGE");

    // -------------------------------------------------------
    // Layer I
    // -------------------------------------------------------
    // Iteration variables
    var i_f("i_f", 0, 2000), j_f("j_f", 0, 2000);
    var t("t", 0, 500), i("i", 1, 2000 - 1), j("j", 1, 2000 - 1);

    // Inputs
    input A("A", {i_f, j_f}, p_float64);

    // Computations
    computation comp_A_out("comp_A_out", {t, i, j}, (A(i-1, j-1) + A(i-1, j) + A(i-1, j+1)
                                                   + A(i, j-1)   + A(i, j)   + A(i, j+1)
                                                   + A(i+1, j-1) + A(i+1, j) + A(i+1, j+1))/9.0);

    // -------------------------------------------------------
    // Layer III
    // -------------------------------------------------------
    // Buffers
    buffer b_A("b_A", {2000, 2000}, p_float64, a_output);

    // Store inputs
    A.store_in(&b_A);

    // Store computations (in place: Gauss-Seidel)
    comp_A_out.store_in(&b_A, {i, j});


    // -------------------------------------------------------
    // Legality tests
    // -------------------------------------------------------
    tiramisu::performe_full_dependency_analysis();

    tiramisu::function *fct = tiramisu::global::get_implicit_function();

    assert(tiramisu::check_legality_of_function() == true);
    assert(fct->check_partial_legality_in_function({&comp_A_out}) == true);



    fct->skewing_local_3D_solver_positive({&comp_A_out}, t, i, j);

    // -------------------------------------------------------
    // Code Generation
    // -------------------------------------------------------
    // Writes function.o and the header function.o.h in the current dir (build/)
    tiramisu::codegen({&b_A}, "function.o");

    return 0;
}