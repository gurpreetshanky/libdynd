//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

#include "inc_gtest.hpp"

#include <dynd/array.hpp>
#include <dynd/kernels/reduction_kernels.hpp>
#include <dynd/func/reduction.hpp>
#include <dynd/func/sum.hpp>
#include <dynd/json_parser.hpp>

#include "dynd_assertions.hpp"

using namespace std;
using namespace dynd;

TEST(Reduction, BuiltinSum_Kernel)
{
  ckernel_builder<kernel_request_host> ckb;
  expr_single_t fn;
  char *src = NULL;

  // int32
  kernels::make_builtin_sum_reduction_ckernel(&ckb, 0, int32_type_id,
                                              kernel_request_single);
  fn = ckb.get()->get_function<expr_single_t>();
  int32_t s32 = 0, a32[3] = {1, -2, 12};
  src = (char *)&a32[0];
  fn(ckb.get(), (char *)&s32, &src);
  EXPECT_EQ(1, s32);
  src = (char *)&a32[1];
  fn(ckb.get(), (char *)&s32, &src);
  EXPECT_EQ(-1, s32);
  src = (char *)&a32[2];
  fn(ckb.get(), (char *)&s32, &src);
  EXPECT_EQ(11, s32);

  // int64
  ckb.reset();
  kernels::make_builtin_sum_reduction_ckernel(&ckb, 0, int64_type_id,
                                              kernel_request_single);
  fn = ckb.get()->get_function<expr_single_t>();
  int64_t s64 = 0, a64[3] = {1, -20000000000LL, 12};
  src = (char *)&a64[0];
  fn(ckb.get(), (char *)&s64, &src);
  EXPECT_EQ(1, s64);
  src = (char *)&a64[1];
  fn(ckb.get(), (char *)&s64, &src);
  EXPECT_EQ(-19999999999LL, s64);
  src = (char *)&a64[2];
  fn(ckb.get(), (char *)&s64, &src);
  EXPECT_EQ(-19999999987LL, s64);

  // float32
  ckb.reset();
  kernels::make_builtin_sum_reduction_ckernel(&ckb, 0, float32_type_id,
                                              kernel_request_single);
  fn = ckb.get()->get_function<expr_single_t>();
  float sf32 = 0, af32[3] = {1.25f, -2.5f, 12.125f};
  src = (char *)&af32[0];
  fn(ckb.get(), (char *)&sf32, &src);
  EXPECT_EQ(1.25f, sf32);
  src = (char *)&af32[1];
  fn(ckb.get(), (char *)&sf32, &src);
  EXPECT_EQ(-1.25f, sf32);
  src = (char *)&af32[2];
  fn(ckb.get(), (char *)&sf32, &src);
  EXPECT_EQ(10.875f, sf32);

  // float64
  ckb.reset();
  kernels::make_builtin_sum_reduction_ckernel(&ckb, 0, float64_type_id,
                                              kernel_request_single);
  fn = ckb.get()->get_function<expr_single_t>();
  double sf64 = 0, af64[3] = {1.25, -2.5, 12.125};
  src = (char *)&af64[0];
  fn(ckb.get(), (char *)&sf64, &src);
  EXPECT_EQ(1.25, sf64);
  src = (char *)&af64[1];
  fn(ckb.get(), (char *)&sf64, &src);
  EXPECT_EQ(-1.25, sf64);
  src = (char *)&af64[2];
  fn(ckb.get(), (char *)&sf64, &src);
  EXPECT_EQ(10.875, sf64);

  // complex[float32]
  ckb.reset();
  kernels::make_builtin_sum_reduction_ckernel(&ckb, 0, complex_float32_type_id,
                                              kernel_request_single);
  fn = ckb.get()->get_function<expr_single_t>();
  dynd::complex<float> scf32 = 0,
                       acf32[3] = {dynd::complex<float>(1.25f, -2.125f),
                                   dynd::complex<float>(-2.5f, 1.0f),
                                   dynd::complex<float>(12.125f, 12345.f)};
  src = (char *)&acf32[0];
  fn(ckb.get(), (char *)&scf32, &src);
  EXPECT_EQ(dynd::complex<float>(1.25f, -2.125f), scf32);
  src = (char *)&acf32[1];
  fn(ckb.get(), (char *)&scf32, &src);
  EXPECT_EQ(dynd::complex<float>(-1.25f, -1.125f), scf32);
  src = (char *)&acf32[2];
  fn(ckb.get(), (char *)&scf32, &src);
  EXPECT_EQ(dynd::complex<float>(10.875f, 12343.875f), scf32);

  // complex[float64]
  ckb.reset();
  kernels::make_builtin_sum_reduction_ckernel(&ckb, 0, complex_float64_type_id,
                                              kernel_request_single);
  fn = ckb.get()->get_function<expr_single_t>();
  dynd::complex<double> scf64 = 0,
                        acf64[3] = {dynd::complex<double>(1.25, -2.125),
                                    dynd::complex<double>(-2.5, 1.0),
                                    dynd::complex<double>(12.125, 12345.)};
  src = (char *)&acf64[0];
  fn(ckb.get(), (char *)&scf64, &src);
  EXPECT_EQ(dynd::complex<float>(1.25, -2.125), scf64);
  src = (char *)&acf64[1];
  fn(ckb.get(), (char *)&scf64, &src);
  EXPECT_EQ(dynd::complex<double>(-1.25, -1.125), scf64);
  src = (char *)&acf64[2];
  fn(ckb.get(), (char *)&scf64, &src);
  EXPECT_EQ(dynd::complex<double>(10.875, 12343.875), scf64);
}

TEST(Reduction, BuiltinSum_Lift0D_NoIdentity)
{
  // Lift it to a zero-dimensional reduction callable (basically a no-op)
  nd::callable f = nd::functional::reduction(
      kernels::make_builtin_sum_reduction_callable(float32_type_id),
      ndt::type("float32"), false, {}, nd::array(),
      commutative | left_associative);

  // Set up some data for the test reduction
  nd::array a = 1.25f;
  ASSERT_EQ(f.get_type()->get_pos_type(0), a.get_type());
  ASSERT_EQ(f.get_type()->get_return_type(), ndt::type::make<float>());
  EXPECT_EQ(1.25f, f(a));
}

TEST(Reduction, BuiltinSum_Lift0D_WithIdentity)
{
  // Lift it to a zero-dimensional reduction callable (basically a no-op)
  // Use 100.f as the "identity" to confirm it's really being used
  nd::callable f = nd::functional::reduction(
      kernels::make_builtin_sum_reduction_callable(float32_type_id),
      ndt::type("float32"), false, {}, nd::array(100.f),
      commutative | left_associative);

  // Set up some data for the test reduction
  nd::array a = 1.25f;
  ASSERT_EQ(f.get_type()->get_pos_type(0), a.get_type());
  ASSERT_EQ(f.get_type()->get_return_type(), ndt::type::make<float>());
  EXPECT_EQ(100.f + 1.25f, f(1.25f));
}

TEST(Reduction, BuiltinSum_Lift1D_NoIdentity)
{
  // Lift it to a one-dimensional strided float32 reduction callable
  nd::callable f = nd::functional::reduction(
      kernels::make_builtin_sum_reduction_callable(float32_type_id),
      ndt::type("Fixed * float32"), false, {0}, nd::array(),
      commutative | left_associative);

  // Set up some data for the test reduction
  float vals0[5] = {1.5, -22., 3.75, 1.125, -3.375};
  nd::array a = vals0;
  EXPECT_TYPE_MATCH(f.get_type()->get_pos_type(0), a.get_type());
  EXPECT_TYPE_MATCH(f.get_type()->get_return_type(), ndt::type::make<float>());

  // Call it on the data
  EXPECT_ARR_EQ(vals0[0] + vals0[1] + vals0[2] + vals0[3] + vals0[4], f(a));

  // Instantiate it again with some different data
  float vals1[1] = {3.75f};
  a = vals1;
  EXPECT_ARR_EQ(vals1[0], f(a));
}

TEST(Reduction, BuiltinSum_Lift1D_WithIdentity)
{
  // Lift it to a one-dimensional strided float32 reduction callable
  // Use 100.f as the "identity" to confirm it's really being used
  nd::callable f = nd::functional::reduction(
      kernels::make_builtin_sum_reduction_callable(float32_type_id),
      ndt::type("Fixed * float32"), false, {0},
      nd::array(100.f), commutative | left_associative);

  // Set up some data for the test reduction
  float vals0[5] = {1.5, -22., 3.75, 1.125, -3.375};
  nd::array a = vals0;
  EXPECT_TYPE_MATCH(f.get_type()->get_pos_type(0), a.get_type());
  EXPECT_TYPE_MATCH(f.get_type()->get_return_type(), ndt::type::make<float>());

  // Call it on the data
  EXPECT_ARR_EQ(100.f + vals0[0] + vals0[1] + vals0[2] + vals0[3] + vals0[4],
                f(a));
}

TEST(Reduction, BuiltinSum_Lift2D_StridedStrided_ReduceReduce)
{
  // Lift it to a two-dimensional strided float32 reduction callable
  nd::callable f = nd::functional::reduction(
      kernels::make_builtin_sum_reduction_callable(float32_type_id),
      ndt::type("Fixed * Fixed * float32"), false, {0, 1},
      nd::array(), commutative | left_associative);

  // Set up some data for the test reduction
  nd::array a =
      parse_json("2 * 3 * float32", "[[1.5, 2, 7], [-2.25, 7, 2.125]]");
  EXPECT_TYPE_MATCH(f.get_type()->get_pos_type(0), a.get_type());
  EXPECT_TYPE_MATCH(f.get_type()->get_return_type(), ndt::type::make<float>());

  // Call it on the data
  EXPECT_ARR_EQ(1.5f + 2.f + 7.f - 2.25f + 7.f + 2.125f, f(a));

  // Instantiate it again with some different data
  a = parse_json("1 * 2 * float32", "[[1.5, -2]]");

  // Call it on the data
  EXPECT_ARR_EQ(1.5f - 2.f, f(a));
}

TEST(Reduction, BuiltinSum_Lift2D_StridedStrided_ReduceReduce_KeepDim)
{
  // Lift it to a two-dimensional strided float32 reduction callable
  nd::callable f = nd::functional::reduction(
      kernels::make_builtin_sum_reduction_callable(float32_type_id),
      ndt::type("Fixed * Fixed * float32"), true, {0, 1},
      nd::array(), commutative | left_associative);

  // Set up some data for the test reduction
  nd::array a =
      parse_json("2 * 3 * float32", "[[1.5, 2, 7], [-2.25, 7, 2.125]]");
  EXPECT_TYPE_MATCH(f.get_type()->get_pos_type(0), a.get_type());
  EXPECT_TYPE_MATCH(f.get_type()->get_return_type(),
                    ndt::type("1 * 1 * float32"));

  // Call it on the data
  EXPECT_ARR_EQ(1.5f + 2.f + 7.f - 2.25f + 7.f + 2.125f,
                f(a)(0, 0).as<float>());
}

TEST(Reduction, BuiltinSum_Lift2D_StridedStrided_BroadcastReduce)
{
  // Lift it to a two-dimensional strided float32 reduction callable
  nd::callable f = nd::functional::reduction(
      kernels::make_builtin_sum_reduction_callable(float32_type_id),
      ndt::type("Fixed * Fixed * float32"), false, {1},
      nd::array(), commutative | left_associative);

  // Set up some data for the test reduction
  nd::array a =
      parse_json("2 * 3 * float32", "[[1.5, 2, 7], [-2.25, 7, 2.125]]");
  EXPECT_TYPE_MATCH(f.get_type()->get_pos_type(0), a.get_type());
  EXPECT_TYPE_MATCH(f.get_type()->get_return_type(), ndt::type("2 * float32"));

  // Call it on the data
  EXPECT_EQ(1.5f + 2.f + 7.f, f(a)(0).as<float>());
  EXPECT_EQ(-2.25f + 7 + 2.125f, f(a)(1).as<float>());

  // Instantiate it again with some different data
  a = parse_json("1 * 2 * float32", "[[1.5, -2]]");

  // Call it on the data
  EXPECT_ARR_EQ(1.5f - 2.f, f(a)(0).as<float>());
}

TEST(Reduction, BuiltinSum_Lift2D_StridedStrided_BroadcastReduce_KeepDim)
{
  // Lift it to a two-dimensional strided float32 reduction callable
  nd::callable f = nd::functional::reduction(
      kernels::make_builtin_sum_reduction_callable(float32_type_id),
      ndt::type("Fixed * Fixed * float32"), true, {1},
      nd::array(), commutative | left_associative);

  // Set up some data for the test reduction
  nd::array a =
      parse_json("2 * 3 * float32", "[[1.5, 2, 7], [-2.25, 7, 2.125]]");
  EXPECT_TYPE_MATCH(f.get_type()->get_pos_type(0), a.get_type());
  EXPECT_TYPE_MATCH(f.get_type()->get_return_type(),
                    ndt::type("2 * 1 * float32"));

  // Call it on the data
  EXPECT_EQ(1.5f + 2.f + 7.f, f(a)(0, 0).as<float>());
  EXPECT_EQ(-2.25f + 7 + 2.125f, f(a)(1, 0).as<float>());
}

TEST(Reduction, BuiltinSum_Lift2D_StridedStrided_ReduceBroadcast)
{
  // Lift it to a two-dimensional strided float32 reduction callable
  nd::callable f = nd::functional::reduction(
      kernels::make_builtin_sum_reduction_callable(float32_type_id),
      ndt::type("Fixed * Fixed * float32"), false, {0},
      nd::array(), commutative | left_associative);

  // Set up some data for the test reduction
  nd::array a =
      parse_json("2 * 3 * float32", "[[1.5, 2, 7], [-2.25, 7, 2.125]]");
  EXPECT_TYPE_MATCH(f.get_type()->get_pos_type(0), a.get_type());
  EXPECT_TYPE_MATCH(f.get_type()->get_return_type(), ndt::type("3 * float32"));

  // Call it on the data
  EXPECT_EQ(1.5f - 2.25f, f(a)(0).as<float>());
  EXPECT_EQ(2.f + 7.f, f(a)(1).as<float>());
  EXPECT_EQ(7.f + 2.125f, f(a)(2).as<float>());

  // Instantiate it again with some different data
  a = parse_json("1 * 2 * float32", "[[1.5, -2]]");
  // Call it on the data
  EXPECT_EQ(1.5f, f(a)(0).as<float>());
  EXPECT_EQ(-2.f, f(a)(1).as<float>());
}

TEST(Reduction, BuiltinSum_Lift2D_StridedStrided_ReduceBroadcast_KeepDim)
{
  // Lift it to a two-dimensional strided float32 reduction callable
  nd::callable f = nd::functional::reduction(
      kernels::make_builtin_sum_reduction_callable(float32_type_id),
      ndt::type("Fixed * Fixed * float32"), true, {0},
      nd::array(), commutative | left_associative);

  // Set up some data for the test reduction
  nd::array a =
      parse_json("2 * 3 * float32", "[[1.5, 2, 7], [-2.25, 7, 2.125]]");
  EXPECT_TYPE_MATCH(f.get_type()->get_pos_type(0), a.get_type());
  EXPECT_TYPE_MATCH(f.get_type()->get_return_type(),
                    ndt::type("1 * 3 * float32"));

  // Call it on the data
  EXPECT_EQ(1.5f - 2.25f, f(a)(0, 0).as<float>());
  EXPECT_EQ(2.f + 7.f, f(a)(0, 1).as<float>());
  EXPECT_EQ(7.f + 2.125f, f(a)(0, 2).as<float>());
}

TEST(Reduction, BuiltinSum_Lift3D_StridedStridedStrided_ReduceReduceReduce)
{
  // Lift it to a three-dimensional strided float32 reduction callable
  nd::callable f = nd::functional::reduction(
      kernels::make_builtin_sum_reduction_callable(float32_type_id),
      ndt::type("Fixed * Fixed * Fixed * float32"), false,
      {0, 1, 2}, nd::array(), commutative | left_associative);

  // Set up some data for the test reduction
  nd::array a = parse_json("2 * 3 * 2 * float32", "[[[1.5, -2.375], [2, 1.25], "
                                                  "[7, -0.5]], [[-2.25, 1], "
                                                  "[7, 0], [2.125, 0.25]]]");
  EXPECT_TYPE_MATCH(f.get_type()->get_pos_type(0), a.get_type());
  EXPECT_TYPE_MATCH(f.get_type()->get_return_type(), ndt::type::make<float>());

  // Call it on the data
  EXPECT_EQ(1.5f - 2.375f + 2.f + 1.25f + 7.f - 0.5f - 2.25f + 1.f + 7.f +
                2.125f + 0.25f,
            f(a).as<float>());
}

TEST(Reduction, BuiltinSum_Lift3D_StridedStridedStrided_BroadcastReduceReduce)
{
  // Lift it to a three-dimensional strided float32 reduction callable
  nd::callable f = nd::functional::reduction(
      kernels::make_builtin_sum_reduction_callable(float32_type_id),
      ndt::type("Fixed * Fixed * Fixed * float32"), false,
      {1, 2}, nd::array(), commutative | left_associative);

  // Set up some data for the test reduction
  nd::array a = parse_json("2 * 3 * 2 * float32", "[[[1.5, -2.375], [2, 1.25], "
                                                  "[7, -0.5]], [[-2.25, 1], "
                                                  "[7, 0], [2.125, 0.25]]]");
  EXPECT_TYPE_MATCH(f.get_type()->get_pos_type(0), a.get_type());
  EXPECT_TYPE_MATCH(f.get_type()->get_return_type(), ndt::type("2 * float32"));

  // Call it on the data
  EXPECT_EQ(1.5f - 2.375f + 2.f + 1.25f + 7.f - 0.5f, f(a)(0).as<float>());
  EXPECT_EQ(-2.25f + 1.f + 7.f + 2.125f + 0.25f, f(a)(1).as<float>());
}

TEST(Reduction, BuiltinSum_Lift3D_StridedStridedStrided_ReduceBroadcastReduce)
{
  // Lift it to a three-dimensional strided float32 reduction callable
  nd::callable f = nd::functional::reduction(
      kernels::make_builtin_sum_reduction_callable(float32_type_id),
      ndt::type("Fixed * Fixed * Fixed * float32"), false,
      {0, 2}, nd::array(), commutative | left_associative);

  // Set up some data for the test reduction
  nd::array a = parse_json("2 * 3 * 2 * float32", "[[[1.5, -2.375], [2, 1.25], "
                                                  "[7, -0.5]], [[-2.25, 1], "
                                                  "[7, 0], [2.125, 0.25]]]");
  a = a(irange(), irange(), irange());
  EXPECT_TYPE_MATCH(f.get_type()->get_pos_type(0), a.get_type());
  EXPECT_TYPE_MATCH(f.get_type()->get_return_type(), ndt::type("3 * float32"));

  // Call it on the data
  EXPECT_EQ(1.5f - 2.375f - 2.25f + 1.f, f(a)(0).as<float>());
  EXPECT_EQ(2.f + 1.25f + 7.f, f(a)(1).as<float>());
  EXPECT_EQ(7.f - 0.5f + 2.125f + 0.25f, f(a)(2).as<float>());
}
