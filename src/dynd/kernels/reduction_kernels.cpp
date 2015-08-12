//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/kernels/reduction_kernels.hpp>
#include <dynd/array.hpp>
#include <dynd/types/fixed_dim_kind_type.hpp>
#include <dynd/func/reduction.hpp>
#include <dynd/kernels/base_kernel.hpp>
#include <dynd/kernels/base_virtual_kernel.hpp>
#include <dynd/kernels/sum_kernel.hpp>

using namespace std;
using namespace dynd;

intptr_t kernels::make_builtin_sum_reduction_ckernel(void *ckb,
                                                     intptr_t ckb_offset,
                                                     type_id_t tid,
                                                     kernel_request_t kernreq)
{
  switch (tid) {
  case int32_type_id:
    nd::sum_kernel<int32_type_id>::make(ckb, kernreq, ckb_offset);
    break;
  case int64_type_id:
    nd::sum_kernel<int64_type_id>::make(ckb, kernreq, ckb_offset);
    break;
  case float32_type_id:
    nd::sum_kernel<float32_type_id>::make(ckb, kernreq, ckb_offset);
    break;
  case float64_type_id:
    nd::sum_kernel<float64_type_id>::make(ckb, kernreq, ckb_offset);
    break;
  case complex_float32_type_id:
    nd::sum_kernel<complex_float32_type_id>::make(ckb, kernreq, ckb_offset);
    break;
  case complex_float64_type_id:
    nd::sum_kernel<complex_float64_type_id>::make(ckb, kernreq, ckb_offset);
    break;
  default: {
    stringstream ss;
    ss << "make_builtin_sum_reduction_ckernel: data type ";
    ss << ndt::type(tid) << " is not supported";
    throw type_error(ss.str());
  }
  }

  return ckb_offset;
}

struct builtin_sum_reduction_kernel {
  static intptr_t
  instantiate(char *DYND_UNUSED(static_data), size_t DYND_UNUSED(data_size),
              char *DYND_UNUSED(data), void *ckb, intptr_t ckb_offset,
              const ndt::type &dst_tp, const char *DYND_UNUSED(dst_arrmeta),
              intptr_t DYND_UNUSED(nsrc), const ndt::type *src_tp,
              const char *const *DYND_UNUSED(src_arrmeta),
              kernel_request_t kernreq,
              const eval::eval_context *DYND_UNUSED(ectx),
              const nd::array &DYND_UNUSED(kwds),
              const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    if (dst_tp != src_tp[0]) {
      stringstream ss;
      ss << "dynd sum reduction: the source type, " << src_tp[0]
         << ", does not match the destination type, " << dst_tp;
      throw type_error(ss.str());
    }
    return kernels::make_builtin_sum_reduction_ckernel(
        ckb, ckb_offset, dst_tp.get_type_id(), kernreq);
  }
};

nd::callable kernels::make_builtin_sum_reduction_callable(type_id_t tid)
{
  if (tid < 0 || tid >= builtin_type_id_count) {
    stringstream ss;
    ss << "make_builtin_sum_reduction_ckernel: data type ";
    ss << ndt::type(tid) << " is not supported";
    throw type_error(ss.str());
  }
  return nd::callable::make<builtin_sum_reduction_kernel>(
      ndt::callable_type::make(ndt::type(tid), ndt::type(tid)), tid, 0);
}

nd::callable kernels::make_builtin_sum1d_callable(type_id_t tid)
{
  nd::callable sum_ew = kernels::make_builtin_sum_reduction_callable(tid);
  return nd::functional::reduction(
      sum_ew, ndt::make_fixed_dim_kind(ndt::type(tid)), false, {0}, 0,
      left_associative | commutative);
}

namespace {
struct double_mean1d_ck
    : nd::base_kernel<double_mean1d_ck, kernel_request_host, 1> {
  intptr_t m_minp;
  intptr_t m_src_dim_size, m_src_stride;

  void single(char *dst, char *const *src)
  {
    intptr_t minp = m_minp, countp = 0;
    intptr_t src_dim_size = m_src_dim_size, src_stride = m_src_stride;
    double result = 0;
    char *src_copy = src[0];
    for (intptr_t i = 0; i < src_dim_size; ++i) {
      double v = *reinterpret_cast<double *>(src_copy);
      if (!dynd::isnan(v)) {
        result += v;
        ++countp;
      }
      src_copy += src_stride;
    }
    if (countp >= minp) {
      *reinterpret_cast<double *>(dst) = result / countp;
    } else {
      *reinterpret_cast<double *>(dst) = numeric_limits<double>::quiet_NaN();
    }
  }
};

struct mean1d_kernel {
  static intptr_t
  instantiate(char *static_data, size_t DYND_UNUSED(data_size),
              char *DYND_UNUSED(data), void *ckb, intptr_t ckb_offset,
              const ndt::type &dst_tp, const char *DYND_UNUSED(dst_arrmeta),
              intptr_t DYND_UNUSED(nsrc), const ndt::type *src_tp,
              const char *const *src_arrmeta, kernel_request_t kernreq,
              const eval::eval_context *DYND_UNUSED(ectx),
              const nd::array &DYND_UNUSED(kwds),
              const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    typedef double_mean1d_ck self_type;
    self_type *self = self_type::make(ckb, kernreq, ckb_offset);
    intptr_t src_dim_size, src_stride;
    ndt::type src_el_tp;
    const char *src_el_arrmeta;
    if (!src_tp[0].get_as_strided(src_arrmeta[0], &src_dim_size, &src_stride,
                                  &src_el_tp, &src_el_arrmeta)) {
      stringstream ss;
      ss << "mean1d: could not process type " << src_tp[0];
      ss << " as a strided dimension";
      throw type_error(ss.str());
    }
    if (src_el_tp.get_type_id() != float64_type_id ||
        dst_tp.get_type_id() != float64_type_id) {
      stringstream ss;
      ss << "mean1d: input element type and output type must be "
            "float64, got " << src_el_tp << " and " << dst_tp;
      throw invalid_argument(ss.str());
    }
    self->m_minp = *reinterpret_cast<intptr_t *>(static_data);
    if (self->m_minp <= 0) {
      if (self->m_minp <= -src_dim_size) {
        throw invalid_argument(
            "minp parameter is too large of a negative number");
      }
      self->m_minp += src_dim_size;
    }
    self->m_src_dim_size = src_dim_size;
    self->m_src_stride = src_stride;
    return ckb_offset;
  }
};
} // anonymous namespace

nd::callable kernels::make_builtin_mean1d_callable(type_id_t tid, intptr_t minp)
{
  if (tid != float64_type_id) {
    stringstream ss;
    ss << "make_builtin_mean1d_callable: data type ";
    ss << ndt::type(tid) << " is not supported";
    throw type_error(ss.str());
  }
  return nd::callable::make<mean1d_kernel>(
      ndt::callable_type::make(
          ndt::type::make<double>(),
          ndt::make_fixed_dim_kind(ndt::type::make<double>())),
      minp, 0);
}
