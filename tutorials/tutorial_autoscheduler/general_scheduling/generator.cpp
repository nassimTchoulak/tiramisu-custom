#include <tiramisu/tiramisu.h>

#include <cassert>
#include <string>

using namespace tiramisu;

/**
 * Test shifting, the case of impossible fusion.
 * The buffer values don't matter: we only check the legality assertions.
 * Running ../generator and getting exit code 0 means every assert passed.
 *
 * Note: do not compile with -DNDEBUG, otherwise the asserts (and the
 * legality checks inside them) are removed.
 */

// These used to come from wrapper_test_191.h
#define SIZE1 128

void generate_function(std::string name, int size)
{
    tiramisu::init(name);

    // -------------------------------------------------------
    // Layer I: algorithm
    // -------------------------------------------------------
    // tiramisu::constant N("N", tiramisu::expr((int32_t) size));
    tiramisu::var i("i", 1, 100), j("j", 1, 100);

    tiramisu::input A("A", {i, j}, tiramisu::p_uint8);

    tiramisu::computation B_out("B_out", {i, j},
         A(5, 5));

    /* A(i,0) would not be within the execution range of B_out, so fusion
       with shifting would be possible; not the case for A(i,3). */
    tiramisu::computation A_out("A_out", {i, j}, B_out(i, j) + 5);

    // -------------------------------------------------------
    // Layer II: scheduling
    // -------------------------------------------------------
    B_out.then(A_out, tiramisu::computation::root);

    // -------------------------------------------------------
    // Layer III: buffers
    // -------------------------------------------------------
    tiramisu::buffer b_A("b_A", {100, 100}, tiramisu::p_uint8, tiramisu::a_input);

    A.store_in(&b_A);
    A_out.store_in(&b_A, {i, j});
    B_out.store_in(&b_A, {i, j});

    // -------------------------------------------------------
    // Legality tests
    // -------------------------------------------------------
    tiramisu::performe_full_dependency_analysis();

    tiramisu::function *fct = tiramisu::global::get_implicit_function();

    assert(tiramisu::check_legality_of_function() == true);
    assert(fct->check_partial_legality_in_function({&A_out, &B_out}) == true);

    // Fuse everything at loop level j
    B_out.then(A_out, j);

    tiramisu::prepare_schedules_for_legality_checks(true);

    assert(fct->check_partial_legality_in_function({&A_out, &B_out}) == false);

    // Shifting A_out is impossible because of A(i,3)
    auto shiftings = fct->correcting_loop_fusion_with_shifting({&B_out}, A_out, {i, j});
    assert(shiftings.size() == 0);

	    fct->compute_loop_fusion_with_shifting_and_beta({&B_out}, A_out, {i, j});

    // 2nd try: remove j
    /*
	auto shiftings2 = fct->correcting_loop_fusion_with_shifting({&B_out}, A_out, {i});
    assert(shiftings2.size() > 0);

    for (auto const &tup : shiftings2)
    {
        A_out.shift(std::get<0>(tup), std::get<1>(tup));
    }

    assert(tiramisu::check_legality_of_function() == true);
    assert(fct->check_partial_legality_in_function({&A_out, &B_out}) == true);

    // -------------------------------------------------------
    // Code generation (written in the current dir, i.e. build/)
    // -------------------------------------------------------
	*/
    tiramisu::codegen({&b_A}, "function.o");
}

int main(int argc, char **argv)
{
    generate_function("function_fusion_shift", SIZE1);
    return 0;
}