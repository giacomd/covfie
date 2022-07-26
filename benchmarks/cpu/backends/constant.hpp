/*
 * This file is part of covfie, a part of the ACTS project
 *
 * Copyright (c) 2022 CERN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License,
 * v. 2.0. If a copy of the MPL was not distributed with this file, You can
 * obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <covfie/core/backend/initial/constant.hpp>
#include <covfie/core/field.hpp>
#include <covfie/core/vector.hpp>

template <typename S, std::size_t N, std::size_t M>
struct Constant {
    using backend_t = covfie::backend::constant<
        covfie::vector::vector_d<S, N>,
        covfie::vector::vector_d<S, M>>;

    static covfie::field<backend_t> get_field()
    {
        return covfie::field<backend_t>(typename backend_t::configuration_t{0.f}
        );
    }

    static std::string get_name()
    {
        return "Constant<float" + std::to_string(N) + ", float" +
               std::to_string(M) + ">";
    }
};