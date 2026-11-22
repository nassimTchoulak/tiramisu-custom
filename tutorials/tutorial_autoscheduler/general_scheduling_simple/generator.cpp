#include <tiramisu/tiramisu.h>

using namespace tiramisu;

int main(int argc, char **argv)
{
    tiramisu::init("function_jacobi1d_XLARGE");

    // -------------------------------------------------------
    // Layer I
    // -------------------------------------------------------
    // Iteration variables
    var i_f("i_f", 0, 4000);
    var t("t", 0, 1000), i("i", 1, 4000 - 1);

    // Inputs
    input A("A", {i_f}, p_float64);
    input B("B", {i_f}, p_float64);

    // Computations
    computation comp_B("comp_B", {t, i}, (A(i-1) + A(i) + A(i+1))*0.33333);
    computation comp_A("comp_A", {t, i}, (B(i-1) + B(i) + B(i+1))*0.33333);

    // -------------------------------------------------------
    // Layer II
    // -------------------------------------------------------
    comp_B.then(comp_A, t);

    // -------------------------------------------------------
    // Layer III
    // -------------------------------------------------------
    // Buffers
    buffer b_A("b_A", {4000}, p_float64, a_output);
    buffer b_B("b_B", {4000}, p_float64, a_output);

    // Store inputs
    A.store_in(&b_A);
    B.store_in(&b_B);

    // Store computations
    comp_B.store_in(&b_B, {i});
    comp_A.store_in(&b_A, {i});




    // -------------------------------------------------------
    // Legality tests
    // -------------------------------------------------------
    tiramisu::performe_full_dependency_analysis();

    tiramisu::function *fct = tiramisu::global::get_implicit_function();

    assert(tiramisu::check_legality_of_function() == true);
    assert(fct->check_partial_legality_in_function({&comp_A, &comp_B}) == true);

    // Fuse everything at loop level j
    comp_B.then(comp_A, i);

    tiramisu::prepare_schedules_for_legality_checks(true);

    assert(fct->check_partial_legality_in_function({&comp_A, &comp_B}) == false);



    // 2nd try: remove j
    auto shiftings2 = fct->correcting_loop_fusion_with_shifting({&comp_B}, comp_A, {t, i});
    assert(shiftings2.size() > 0);

    for (auto const &tup : shiftings2)
    {
        comp_A.shift(std::get<0>(tup), std::get<1>(tup));
    }

    assert(tiramisu::check_legality_of_function() == true);
    assert(fct->check_partial_legality_in_function({&comp_A, &comp_B}) == true);


    // -------------------------------------------------------
    // Code Generation
    // -------------------------------------------------------
    // Writes function.o and the header function.o.h in the current dir (build/)
    tiramisu::codegen({&b_A, &b_B}, "function.o");

    return 0;
}