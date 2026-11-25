#include <tiramisu/tiramisu.h>

using namespace tiramisu;

int main(int argc, char **argv)
{
    tiramisu::init("function_heat3d_LARGE");

    // -------------------------------------------------------
    // Layer I
    // -------------------------------------------------------
    // Iteration variables
    var i_f("i_f", 0, 120), j_f("j_f", 0, 120), k_f("k_f", 0, 120);
    var t("t", 1, 500 + 1), i("i", 1, 120 - 1), j("j", 1, 120 - 1), k("k", 1, 120 - 1);

    // Inputs
    input A("A", {i_f, j_f, k_f}, p_float64);
    input B("B", {i_f, j_f, k_f}, p_float64);

    // Computations
    computation B_out("B_out", {t, i, j, k}, (A(i+1, j, k) - A(i, j, k)*2.0 + A(i-1, j, k))*0.125
                                           + (A(i, j+1, k) - A(i, j, k)*2.0 + A(i, j-1, k))*0.125
                                           + (A(i, j, k+1) - A(i, j, k)*2.0 + A(i, j, k-1))*0.125
                                           + A(i, j, k));

    computation A_out("A_out", {t, i, j, k}, (B(i+1, j, k) - B(i, j, k)*2.0 + B(i-1, j, k))*0.125
                                           + (B(i, j+1, k) - B(i, j, k)*2.0 + B(i, j-1, k))*0.125
                                           + (B(i, j, k+1) - B(i, j, k)*2.0 + B(i, j, k-1))*0.125
                                           + B(i, j, k));

    // -------------------------------------------------------
    // Layer II
    // -------------------------------------------------------
    B_out.then(A_out, t);

    // -------------------------------------------------------
    // Layer III
    // -------------------------------------------------------
    // Buffers
    buffer b_A("b_A", {120, 120, 120}, p_float64, a_output);
    buffer b_B("b_B", {120, 120, 120}, p_float64, a_output);

    // Store inputs
    A.store_in(&b_A);
    B.store_in(&b_B);

    // Store computations
    A_out.store_in(&b_A, {i, j, k});
    B_out.store_in(&b_B, {i, j, k});


        // -------------------------------------------------------
    // Legality tests
    // -------------------------------------------------------
    tiramisu::performe_full_dependency_analysis();

    tiramisu::function *fct = tiramisu::global::get_implicit_function();

    assert(tiramisu::check_legality_of_function() == true);
    assert(fct->check_partial_legality_in_function({&A_out, &B_out}) == true);

    // Fuse everything at loop level j
    B_out.then(A_out, k);

    tiramisu::prepare_schedules_for_legality_checks(true);


    auto shiftings2 = fct->correcting_loop_fusion_with_shifting({&B_out}, A_out, {t, i, j, k});
    assert(shiftings2.size() > 0);

    for (auto const &tup : shiftings2)
    {
        A_out.shift(std::get<0>(tup), std::get<1>(tup));
    }
    /*A_out.shift(i, 1);
    A_out.shift(j, 1);
    A_out.shift(k, 1);*/

    assert(tiramisu::check_legality_of_function() == true);
    assert(fct->check_partial_legality_in_function({&A_out, &B_out}) == true);

    fct->skewing_local_4D_solver_positive({&A_out, &B_out}, t, i, j, k);

    // -------------------------------------------------------
    // Code Generation
    // -------------------------------------------------------
    // Writes function.o and the header function.o.h in the current dir (build/)
    tiramisu::codegen({&b_A, &b_B}, "function.o");
    return 0;
}