//
// Copyright (C) 2011-14 Irwin Zaid, Mark Wiebe, DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

#include "inc_gtest.hpp"
#include "dynd_assertions.hpp"

#include <dynd/array.hpp>
#include <dynd/func/multidispatch_arrfunc.hpp>
#include <dynd/func/functor_arrfunc.hpp>
#include <dynd/func/lift_arrfunc.hpp>

using namespace std;
using namespace dynd;

int func0(int, float, double) { return 0; }
int func1(int, double, double) { return 1; }
int func2(int, float, float) { return 2; }
int func3(float, int, int) { return 3; }
int func4(int16_t, float, double) { return 4; }
int func5(int16_t, float, float) { return 5; }

TEST(MultiDispatchArrfunc, Ambiguous) {
  vector<nd::arrfunc> funcs;
  funcs.push_back(nd::make_functor_arrfunc(&func0));
  funcs.push_back(nd::make_functor_arrfunc(&func1));
  funcs.push_back(nd::make_functor_arrfunc(&func2));
  funcs.push_back(nd::make_functor_arrfunc(&func3));
  funcs.push_back(nd::make_functor_arrfunc(&func4));

  EXPECT_THROW(make_multidispatch_arrfunc(funcs.size(), &funcs[0]),
               invalid_argument);

  funcs.push_back(nd::make_functor_arrfunc(&func5));
}

TEST(MultiDispatchArrfunc, ExactSignatures) {
  vector<nd::arrfunc> funcs;
  funcs.push_back(nd::make_functor_arrfunc(&func0));
  funcs.push_back(nd::make_functor_arrfunc(&func1));
  funcs.push_back(nd::make_functor_arrfunc(&func2));
  funcs.push_back(nd::make_functor_arrfunc(&func3));
  funcs.push_back(nd::make_functor_arrfunc(&func4));
  funcs.push_back(nd::make_functor_arrfunc(&func5));

  nd::arrfunc af = make_multidispatch_arrfunc(funcs.size(), &funcs[0]);

  EXPECT_EQ(0, af(1, 1.f, 1.0).as<int>());
  EXPECT_EQ(1, af(1, 1.0, 1.0).as<int>());
  EXPECT_EQ(2, af(1, 1.f, 1.f).as<int>());
  EXPECT_EQ(3, af(1.f, 1, 1).as<int>());
  EXPECT_EQ(4, af((int16_t)1, 1.f, 1.0).as<int>());
  EXPECT_EQ(5, af((int16_t)1, 1.f, 1.f).as<int>());
}

TEST(MultiDispatchArrfunc, PromoteToSignature) {
  vector<nd::arrfunc> funcs;
  funcs.push_back(nd::make_functor_arrfunc(&func0));
  funcs.push_back(nd::make_functor_arrfunc(&func1));
  funcs.push_back(nd::make_functor_arrfunc(&func2));
  funcs.push_back(nd::make_functor_arrfunc(&func3));
  funcs.push_back(nd::make_functor_arrfunc(&func4));
  funcs.push_back(nd::make_functor_arrfunc(&func5));

  nd::arrfunc af = make_multidispatch_arrfunc(funcs.size(), &funcs[0]);

  EXPECT_EQ(0, af(1, dynd_float16(1.f), 1.0).as<int>());
  EXPECT_EQ(1, af(1, 1.0, 1.f).as<int>());
  EXPECT_EQ(2, af(1, 1.f, dynd_float16(1.f)).as<int>());
  EXPECT_EQ(3, af(1.f, 1, (int16_t)1).as<int>());
  EXPECT_EQ(4, af((int8_t)1, 1.f, 1.0).as<int>());
  EXPECT_EQ(5, af((int8_t)1, 1.f, 1.f).as<int>());
}
