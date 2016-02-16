//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <dynd/kernels/base_kernel.hpp>
#include <dynd/types/callable_type.hpp>

namespace dynd {
namespace nd {

  template <type_id_t I0, type_id_t I1>
  struct equal_kernel : base_kernel<equal_kernel<I0, I1>, 2> {
    typedef typename type_of<I0>::type A0;
    typedef typename type_of<I1>::type A1;
    typedef typename std::common_type<A0, A1>::type T;

    void single(char *dst, char *const *src)
    {
      *reinterpret_cast<bool1 *>(dst) =
          static_cast<T>(*reinterpret_cast<A0 *>(src[0])) == static_cast<T>(*reinterpret_cast<A1 *>(src[1]));
    }
  };

  template <type_id_t I0>
  struct equal_kernel<I0, I0> : base_kernel<equal_kernel<I0, I0>, 2> {
    typedef typename type_of<I0>::type A0;

    void single(char *dst, char *const *src)
    {
      *reinterpret_cast<bool1 *>(dst) = *reinterpret_cast<A0 *>(src[0]) == *reinterpret_cast<A0 *>(src[1]);
    }
  };

  template <>
  struct equal_kernel<tuple_id, tuple_id> : base_kernel<equal_kernel<tuple_id, tuple_id>, 2> {
    size_t field_count;
    const size_t *src0_data_offsets, *src1_data_offsets;
    // After this are field_count sorting_less kernel offsets, for
    // src0.field_i <op> src1.field_i
    // with each 0 <= i < field_count

    equal_kernel(size_t field_count, const size_t *src0_data_offsets, const size_t *src1_data_offsets)
        : field_count(field_count), src0_data_offsets(src0_data_offsets), src1_data_offsets(src1_data_offsets)
    {
    }

    ~equal_kernel()
    {
      size_t *kernel_offsets = get_offsets();
      for (size_t i = 0; i != field_count; ++i) {
        get_child(kernel_offsets[i])->destroy();
      }
    }

    size_t *get_offsets() { return reinterpret_cast<size_t *>(this + 1); }

    void single(char *dst, char *const *src)
    {
      const size_t *kernel_offsets = reinterpret_cast<const size_t *>(this + 1);
      char *child_src[2];
      for (size_t i = 0; i != field_count; ++i) {
        kernel_prefix *echild = reinterpret_cast<kernel_prefix *>(reinterpret_cast<char *>(this) + kernel_offsets[i]);
        kernel_single_t opchild = echild->get_function<kernel_single_t>();
        // if (src0.field_i < src1.field_i) return true
        child_src[0] = src[0] + src0_data_offsets[i];
        child_src[1] = src[1] + src1_data_offsets[i];
        bool1 child_dst;
        opchild(echild, reinterpret_cast<char *>(&child_dst), child_src);
        if (!child_dst) {
          *reinterpret_cast<bool1 *>(dst) = false;
          return;
        }
      }
      *reinterpret_cast<bool1 *>(dst) = true;
    }

    static void instantiate(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), kernel_builder *ckb,
                            const ndt::type &DYND_UNUSED(dst_tp), const char *DYND_UNUSED(dst_arrmeta),
                            intptr_t DYND_UNUSED(nsrc), const ndt::type *src_tp, const char *const *src_arrmeta,
                            kernel_request_t DYND_UNUSED(kernreq), intptr_t DYND_UNUSED(nkwd),
                            const nd::array *DYND_UNUSED(kwds),
                            const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars));
  };

} // namespace dynd::nd

namespace ndt {

  template <type_id_t Src0TypeID, type_id_t Src1TypeID>
  struct traits<nd::equal_kernel<Src0TypeID, Src1TypeID>> {
    static type equivalent() { return callable_type::make(make_type<bool1>(), {type(Src0TypeID), type(Src1TypeID)}); }
  };

} // namespace dynd::ndt
} // namespace dynd