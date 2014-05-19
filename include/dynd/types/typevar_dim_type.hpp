//
// Copyright (C) 2011-14 Mark Wiebe, DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#ifndef _DYND__TYPEVAR_DIM_TYPE_HPP_
#define _DYND__TYPEVAR_DIM_TYPE_HPP_

#include <vector>
#include <string>

#include <dynd/array.hpp>
#include <dynd/string.hpp>
#include <dynd/types/base_uniform_dim_type.hpp>

namespace dynd {

class typevar_dim_type : public base_uniform_dim_type {
    nd::string m_name;

public:
    typevar_dim_type(const nd::string &name, const ndt::type &element_type);

    virtual ~typevar_dim_type() {}

    inline const nd::string& get_name() const {
        return m_name;
    }

    inline std::string get_name_str() const {
        return m_name.str();
    }

    void print_data(std::ostream& o, const char *metadata, const char *data) const;

    void print_type(std::ostream& o) const;

    ndt::type apply_linear_index(intptr_t nindices, const irange *indices,
                size_t current_i, const ndt::type& root_tp, bool leading_dimension) const;
    intptr_t apply_linear_index(intptr_t nindices, const irange *indices, const char *metadata,
                    const ndt::type& result_tp, char *out_metadata,
                    memory_block_data *embedded_reference,
                    size_t current_i, const ndt::type& root_tp,
                    bool leading_dimension, char **inout_data,
                    memory_block_data **inout_dataref) const;

    intptr_t get_dim_size(const char *metadata, const char *data) const;

    bool is_lossless_assignment(const ndt::type& dst_tp, const ndt::type& src_tp) const;

    bool operator==(const base_type& rhs) const;

    void metadata_default_construct(char *metadata, intptr_t ndim,
                                    const intptr_t *shape) const;
    void metadata_copy_construct(char *dst_metadata, const char *src_metadata,
                                 memory_block_data *embedded_reference) const;
    size_t
    metadata_copy_construct_onedim(char *dst_metadata, const char *src_metadata,
                                   memory_block_data *embedded_reference) const;
    void metadata_destruct(char *metadata) const;

    void get_dynamic_type_properties(
        const std::pair<std::string, gfunc::callable> **out_properties,
        size_t *out_count) const;
}; // class typevar_dim_type

namespace ndt {
    /** Makes a typevar type with the specified name and element type */
    inline ndt::type make_typevar_dim(const nd::string &name,
                                  const ndt::type &element_type)
    {
        return ndt::type(new typevar_dim_type(name, element_type), false);
    }
} // namespace ndt

} // namespace dynd

#endif // _DYND__TYPEVAR_DIM_TYPE_HPP_