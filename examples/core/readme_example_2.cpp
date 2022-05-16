/*
 * This file is part of covfie, a part of the ACTS project
 *
 * Copyright (c) 2022 CERN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License,
 * v. 2.0. If a copy of the MPL was not distributed with this file, You can
 * obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cmath>
#include <iostream>

#include <covfie/core/backend/builder.hpp>
#include <covfie/core/backend/layout/strided.hpp>
#include <covfie/core/backend/storage/array.hpp>
#include <covfie/core/backend/transformer/interpolator/linear.hpp>
#include <covfie/core/backend/vector/input.hpp>
#include <covfie/core/backend/vector/output.hpp>
#include <covfie/core/field.hpp>
#include <covfie/core/field_view.hpp>

using builder_t = covfie::field<covfie::backend::builder<
    covfie::backend::vector::input::ulong2,
    covfie::backend::vector::output::float2>>;

using field_t =
    covfie::field<covfie::backend::transformer::interpolator::linear<
        covfie::backend::layout::strided<
            covfie::backend::vector::input::ulong2,
            covfie::backend::storage::array<
                covfie::backend::vector::output::float2>>>>;

int main(void)
{
    // Initialize the field as a 10x10 field, then create a view from it.
    builder_t my_field(builder_t::backend_t::configuration_data_t{10ul, 10ul});
    builder_t::view_t my_view(my_field);

    // Assign f(x, y) = (sin x, cos y)
    for (std::size_t x = 0ul; x < 10ul; ++x) {
        for (std::size_t y = 0ul; y < 10ul; ++y) {
            my_view.at(x, y)[0] = std::sin(static_cast<float>(x));
            my_view.at(x, y)[1] = std::cos(static_cast<float>(y));
        }
    }

    field_t new_field(my_field);
    field_t::view_t new_view(new_field);

    // Retrieve the vector value at (2.31, 3.98)
    field_t::output_t v = new_view.at(2.31f, 3.98f);

    std::cout << "Value at (2.31, 3.98) = (" << v[0] << ", " << v[1] << ")"
              << std::endl;

    return 0;
}