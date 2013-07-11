//
// Copyright (C) 2011-13 Mark Wiebe, DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <iostream>
#include <sstream>
#include <stdexcept>
#include "inc_gtest.hpp"

#include <dynd/array.hpp>
#include <dynd/dtypes/type_type.hpp>
#include <dynd/dtypes/strided_dim_type.hpp>
#include <dynd/dtypes/fixed_dim_type.hpp>
#include <dynd/dtypes/var_dim_type.hpp>

using namespace std;
using namespace dynd;

TEST(DTypeDType, Create) {
    ndt::type d;

    // Strings with various encodings and sizes
    d = ndt::make_type();
    EXPECT_EQ(dtype_type_id, d.get_type_id());
    EXPECT_EQ(custom_kind, d.get_kind());
    EXPECT_EQ(ndt::type("type"), d);
    EXPECT_EQ(sizeof(const base_type *), d.get_data_alignment());
    EXPECT_EQ(sizeof(const base_type *), d.get_data_size());
    EXPECT_FALSE(d.is_expression());
}

TEST(DTypeDType, BasicNDobject) {
    nd::array a;

    a = ndt::type("int32");
    EXPECT_EQ(dtype_type_id, a.get_type().get_type_id());
    EXPECT_EQ(ndt::make_type<int32_t>(), a.as<ndt::type>());
}

TEST(DTypeDType, StringCasting) {
    nd::array a;

    a = nd::array("int32").ucast(ndt::make_type());
    a = a.eval();
    EXPECT_EQ(dtype_type_id, a.get_type().get_type_id());
    EXPECT_EQ(ndt::make_type<int32_t>(), a.as<ndt::type>());
    EXPECT_EQ("int32", a.as<string>());
}

TEST(DTypeDType, ScalarRefCount) {
    nd::array a;
    ndt::type d, d2;
    d = ndt::make_strided_dim(ndt::make_type<int>());

    a = nd::empty(ndt::make_type());
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals() = d;
    EXPECT_EQ(2, d.extended()->get_use_count());
    d2 = a.as<ndt::type>();
    EXPECT_EQ(3, d.extended()->get_use_count());
    d2 = ndt::type();
    EXPECT_EQ(2, d.extended()->get_use_count());
    // Assigning a new value in the nd::array should free the reference in 'a'
    a.vals() = ndt::type();
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals() = d;
    EXPECT_EQ(2, d.extended()->get_use_count());
    // Assigning a new reference to 'a' should free the reference when
    // destructing the existing 'a'
    a = 1.0;
    EXPECT_EQ(1, d.extended()->get_use_count());
}

TEST(DTypeDType, StridedArrayRefCount) {
    nd::array a;
    ndt::type d;
    d = ndt::make_strided_dim(ndt::make_type<int>());

    // 1D Strided Array
    a = nd::empty(10, ndt::make_strided_dim(ndt::make_type()));
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals() = d;
    EXPECT_EQ(11, d.extended()->get_use_count());
    // Assigning one value should free one reference count
    a.vals_at(0) = ndt::make_type<float>();
    EXPECT_EQ(10, d.extended()->get_use_count());
    // Assigning all values should free all the reference counts
    a.vals() = ndt::make_type<int>();
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals() = d;
    EXPECT_EQ(11, d.extended()->get_use_count());
    // Assigning a new reference to 'a' should free the references when
    // destructing the existing 'a'
    a = 1.0;
    EXPECT_EQ(1, d.extended()->get_use_count());

    // 2D Strided Array
    a = nd::empty(3, 3, ndt::type("M, N, type"));
    EXPECT_EQ(strided_dim_type_id, a.get_type().get_type_id());
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals() = d;
    EXPECT_EQ(10, d.extended()->get_use_count());
    // Assigning one value should free one reference count
    a.vals_at(0,1) = ndt::make_type<float>();
    EXPECT_EQ(9, d.extended()->get_use_count());
    // Assigning all values should free all the reference counts
    a.vals() = ndt::make_type<int>();
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals() = d;
    EXPECT_EQ(10, d.extended()->get_use_count());
    // Assigning one slice should free several reference counts
    a.vals_at(1) = ndt::make_type<double>();
    EXPECT_EQ(7, d.extended()->get_use_count());
    // Assigning a new reference to 'a' should free the references when
    // destructing the existing 'a'
    a = 1.0;
    EXPECT_EQ(1, d.extended()->get_use_count());
}


TEST(DTypeDType, FixedArrayRefCount) {
    nd::array a;
    ndt::type d;
    d = ndt::make_strided_dim(ndt::make_type<int>());

    // 1D Fixed Array
    a = nd::empty(ndt::make_fixed_dim(10, ndt::make_type()));
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals() = d;
    EXPECT_EQ(11, d.extended()->get_use_count());
    // Assigning one value should free one reference count
    a.vals_at(0) = ndt::make_type<float>();
    EXPECT_EQ(10, d.extended()->get_use_count());
    // Assigning all values should free all the reference counts
    a.vals() = ndt::make_type<int>();
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals() = d;
    EXPECT_EQ(11, d.extended()->get_use_count());
    // Assigning a new reference to 'a' should free the references when
    // destructing the existing 'a'
    a = 1.0;
    EXPECT_EQ(1, d.extended()->get_use_count());

    // 2D Fixed Array
    a = nd::empty(ndt::type("3, 3, type"));
    EXPECT_EQ(fixed_dim_type_id, a.get_type().get_type_id());
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals() = d;
    EXPECT_EQ(10, d.extended()->get_use_count());
    // Assigning one value should free one reference count
    a.vals_at(0,1) = ndt::make_type<float>();
    EXPECT_EQ(9, d.extended()->get_use_count());
    // Assigning all values should free all the reference counts
    a.vals() = ndt::make_type<int>();
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals() = d;
    EXPECT_EQ(10, d.extended()->get_use_count());
    // Assigning one slice should free several reference counts
    a.vals_at(1) = ndt::make_type<double>();
    EXPECT_EQ(7, d.extended()->get_use_count());
    // Assigning a new reference to 'a' should free the references when
    // destructing the existing 'a'
    a = 1.0;
    EXPECT_EQ(1, d.extended()->get_use_count());
}

TEST(DTypeDType, VarArrayRefCount) {
    nd::array a;
    ndt::type d;
    d = ndt::make_strided_dim(ndt::make_type<int>());

    // 1D Var Array
    a = nd::empty(ndt::make_var_dim(ndt::make_type()));
    // It should have an objectarray memory block type
    EXPECT_EQ((uint32_t)objectarray_memory_block_type,
                    reinterpret_cast<const var_dim_type_metadata *>(
                        a.get_ndo_meta())->blockref->m_type);
    a.vals() = nd::empty("10, type");
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals() = d;
    EXPECT_EQ(11, d.extended()->get_use_count());
    // Assigning one value should free one reference count
    a(0).vals() = ndt::make_type<float>();
    EXPECT_EQ(10, d.extended()->get_use_count());
    // Assigning all values should free all the reference counts
    a.vals() = ndt::make_type<int>();
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals() = d;
    EXPECT_EQ(11, d.extended()->get_use_count());
    // Assigning a new reference to 'a' should free the references when
    // destructing the existing 'a'
    a = 1.0;
    EXPECT_EQ(1, d.extended()->get_use_count());

    // 2D Strided + Var Array
    a = nd::empty(3, ndt::make_strided_dim(ndt::make_var_dim(ndt::make_type())));
    a.vals_at(0) = nd::empty("2, type");
    a.vals_at(1) = nd::empty("3, type");
    a.vals_at(2) = nd::empty("4, type");
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals() = d;
    EXPECT_EQ(10, d.extended()->get_use_count());
    // Assigning one value should free one reference count
    a(0,1).vals() = ndt::make_type<float>();
    EXPECT_EQ(9, d.extended()->get_use_count());
    // Assigning all values should free all the reference counts
    a.vals() = ndt::make_type<int>();
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals() = d;
    EXPECT_EQ(10, d.extended()->get_use_count());
    // Assigning one slice should free several reference counts
    a.vals_at(2) = ndt::make_type<double>();
    EXPECT_EQ(6, d.extended()->get_use_count());
    // Assigning a new reference to 'a' should free the references when
    // destructing the existing 'a'
    a = 1.0;
    EXPECT_EQ(1, d.extended()->get_use_count());
}

TEST(DTypeDType, CStructRefCount) {
    nd::array a;
    ndt::type d;
    d = ndt::make_strided_dim(ndt::make_type<int>());

    // Single CStruct Instance
    a = nd::empty("{dt: type; more: {a: int32; b: type}; other: string}");
    EXPECT_EQ(cstruct_type_id, a.get_type().get_type_id());
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.p("dt").vals() = d;
    EXPECT_EQ(2, d.extended()->get_use_count());
    a.p("more").p("b").vals() = d;
    EXPECT_EQ(3, d.extended()->get_use_count());
    // Assigning one value should free one reference count
    a.vals_at(0) = ndt::type();
    EXPECT_EQ(2, d.extended()->get_use_count());
    a.vals_at(1,1) = ndt::make_type<int>();
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals_at(0) = d;
    EXPECT_EQ(2, d.extended()->get_use_count());
    a.vals_at(1,1) = d;
    EXPECT_EQ(3, d.extended()->get_use_count());
    // Assigning a new reference to 'a' should free the references when
    // destructing the existing 'a'
    a = 1.0;
    EXPECT_EQ(1, d.extended()->get_use_count());

    // Array of CStruct Instance
    a = nd::empty(10, "M, {dt: type; more: {a: int32; b: type}; other: string}");
    EXPECT_EQ(cstruct_type_id, a(0).get_type().get_type_id());
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.p("dt").vals() = d;
    EXPECT_EQ(11, d.extended()->get_use_count());
    a.p("more").p("b").vals() = d;
    EXPECT_EQ(21, d.extended()->get_use_count());
    // Assigning one value should free one reference count
    a.vals_at(0,0) = ndt::type();
    EXPECT_EQ(20, d.extended()->get_use_count());
    a.vals_at(-1,1,1) = ndt::make_type<int>();
    EXPECT_EQ(19, d.extended()->get_use_count());
    // Assigning one slice should free several reference counts
    a(3 <= irange() < 6).p("dt").vals() = ndt::type();
    EXPECT_EQ(16, d.extended()->get_use_count());
    // Assigning a new reference to 'a' should free the references when
    // destructing the existing 'a'
    a = 1.0;
    EXPECT_EQ(1, d.extended()->get_use_count());
}


TEST(DTypeDType, StructRefCount) {
    nd::array a;
    ndt::type d;
    d = ndt::make_strided_dim(ndt::make_type<int>());

    // Single CStruct Instance
    a = nd::empty("{dt: type; more: {a: int32; b: type}; other: string}")(0 <= irange() < 2);
    EXPECT_EQ(struct_type_id, a.get_type().get_type_id());
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.p("dt").vals() = d;
    EXPECT_EQ(2, d.extended()->get_use_count());
    a.p("more").p("b").vals() = d;
    EXPECT_EQ(3, d.extended()->get_use_count());
    // Assigning one value should free one reference count
    a.vals_at(0) = ndt::type();
    EXPECT_EQ(2, d.extended()->get_use_count());
    a.vals_at(1,1) = ndt::make_type<int>();
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.vals_at(0) = d;
    EXPECT_EQ(2, d.extended()->get_use_count());
    a.vals_at(1,1) = d;
    EXPECT_EQ(3, d.extended()->get_use_count());
    // Assigning a new reference to 'a' should free the references when
    // destructing the existing 'a'
    a = 1.0;
    EXPECT_EQ(1, d.extended()->get_use_count());

    // Array of Struct Instance
    a = nd::empty(10, "M, {dt: type; more: {a: int32; b: type}; other: string}")(irange(), 0 <= irange() < 2);
    EXPECT_EQ(struct_type_id, a(0).get_type().get_type_id());
    EXPECT_EQ(1, d.extended()->get_use_count());
    a.p("dt").vals() = d;
    EXPECT_EQ(11, d.extended()->get_use_count());
    a.p("more").p("b").vals() = d;
    EXPECT_EQ(21, d.extended()->get_use_count());
    // Assigning one value should free one reference count
    a.vals_at(0,0) = ndt::type();
    EXPECT_EQ(20, d.extended()->get_use_count());
    a.vals_at(-1,1,1) = ndt::make_type<int>();
    EXPECT_EQ(19, d.extended()->get_use_count());
    // Assigning one slice should free several reference counts
    a(3 <= irange() < 6).p("dt").vals() = ndt::type();
    EXPECT_EQ(16, d.extended()->get_use_count());
    // Assigning a new reference to 'a' should free the references when
    // destructing the existing 'a'
    a = 1.0;
    EXPECT_EQ(1, d.extended()->get_use_count());
}