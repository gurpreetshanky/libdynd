//
// Copyright (C) 2011-12, Dynamic NDArray Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <algorithm>

#include <dynd/dtypes/fixedbytes_dtype.hpp>
#include <dynd/kernels/single_compare_kernel_instance.hpp>
#include <dynd/kernels/assignment_kernels.hpp>
#include <dynd/shape_tools.hpp>
#include <dynd/exceptions.hpp>

using namespace std;
using namespace dynd;

dynd::fixedbytes_dtype::fixedbytes_dtype(intptr_t element_size, intptr_t alignment)
    : m_element_size(element_size), m_alignment(alignment)
{
    if (alignment > element_size) {
        std::stringstream ss;
        ss << "Cannot make a fixedbytes<" << element_size << "," << alignment << "> dtype, its alignment is greater than its size";
        throw std::runtime_error(ss.str());
    }
    if (alignment != 1 && alignment != 2 && alignment != 4 && alignment != 8 && alignment != 16) {
        std::stringstream ss;
        ss << "Cannot make a fixedbytes<" << element_size << "," << alignment << "> dtype, its alignment is not a small power of two";
        throw std::runtime_error(ss.str());
    }
    if ((element_size&(alignment-1)) != 0) {
        std::stringstream ss;
        ss << "Cannot make a fixedbytes<" << element_size << "," << alignment << "> dtype, its alignment does not divide into its element size";
        throw std::runtime_error(ss.str());
    }
}

void dynd::fixedbytes_dtype::print_element(std::ostream& o, const char *data, const char *DYND_UNUSED(metadata)) const
{
    o << "0x";
    hexadecimal_print(o, data, m_element_size);
}

void dynd::fixedbytes_dtype::print_dtype(std::ostream& o) const
{
    o << "fixedbytes<" << m_element_size << "," << m_alignment << ">";
}

dtype dynd::fixedbytes_dtype::apply_linear_index(int nindices, const irange *indices, int current_i, const dtype& root_dt) const
{
    if (nindices == 0) {
        return dtype(this);
    } else if (nindices == 1) {
        if (indices->step() == 0) {
            // If the string encoding is variable-length switch to UTF32 so that the result can always
            // store a single character.
            return make_fixedbytes_dtype(1, 1);
        } else {
            // Get the size of the type after applying the index
            bool remove_dimension;
            intptr_t start_index, index_stride, dimension_size;
            apply_single_linear_index(*indices, m_element_size, current_i, &root_dt, remove_dimension, start_index, index_stride, dimension_size);
            return make_fixedbytes_dtype(dimension_size, 1);
        }
    } else {
        throw too_many_indices(nindices, current_i + 1);
    }
}

void dynd::fixedbytes_dtype::get_single_compare_kernel(single_compare_kernel_instance& DYND_UNUSED(out_kernel)) const {
    throw std::runtime_error("fixedbytes_dtype::get_single_compare_kernel not supported yet");
}

bool dynd::fixedbytes_dtype::is_lossless_assignment(const dtype& dst_dt, const dtype& src_dt) const
{
    if (dst_dt.extended() == this) {
        if (src_dt.extended() == this) {
            return true;
        } else if (src_dt.type_id() == fixedbytes_type_id) {
            const fixedbytes_dtype *src_fs = static_cast<const fixedbytes_dtype*>(src_dt.extended());
            return m_element_size == src_fs->m_element_size;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void dynd::fixedbytes_dtype::get_dtype_assignment_kernel(const dtype& dst_dt, const dtype& src_dt,
                assign_error_mode errmode,
                unary_specialization_kernel_instance& out_kernel) const
{
    if (this == dst_dt.extended()) {
        switch (src_dt.type_id()) {
            case fixedbytes_type_id: {
                const fixedbytes_dtype *src_fs = static_cast<const fixedbytes_dtype *>(src_dt.extended());
                if (m_element_size != src_fs->m_element_size) {
                    throw runtime_error("cannot assign to a fixedbytes dtype of a different size");
                }
                get_pod_dtype_assignment_kernel(m_element_size, std::min(m_alignment, src_fs->m_alignment), out_kernel);
                break;
            }
            default: {
                src_dt.extended()->get_dtype_assignment_kernel(dst_dt, src_dt, errmode, out_kernel);
                break;
            }
        }
    } else {
        throw runtime_error("conversions from bytes to non-bytes are not implemented");
    }
}


bool dynd::fixedbytes_dtype::operator==(const extended_dtype& rhs) const
{
    if (this == &rhs) {
        return true;
    } else if (rhs.type_id() != fixedbytes_type_id) {
        return false;
    } else {
        const fixedbytes_dtype *dt = static_cast<const fixedbytes_dtype*>(&rhs);
        return m_element_size == dt->m_element_size && m_alignment == dt->m_alignment;
    }
}
