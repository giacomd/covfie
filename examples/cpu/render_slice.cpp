/*
 * This file is part of covfie, a part of the ACTS project
 *
 * Copyright (c) 2022 CERN
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License,
 * v. 2.0. If a copy of the MPL was not distributed with this file, You can
 * obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <chrono>
#include <cstddef>
#include <fstream>
#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include <covfie/core/backend/transformer/affine.hpp>
#include <covfie/core/backend/transformer/interpolator/linear.hpp>
#include <covfie/core/backend/transformer/layout/strided.hpp>
#include <covfie/core/field.hpp>

#include "bitmap.hpp"

void parse_opts(
    int argc, char * argv[], boost::program_options::variables_map & vm
)
{
    boost::program_options::options_description opts("general options");

    opts.add_options()("help", "produce help message")(
        "input,i",
        boost::program_options::value<std::string>()->required(),
        "input vector field to read"
    )("output,o",
      boost::program_options::value<std::string>()->required(),
      "output bitmap image to write"
    )("height,h",
      boost::program_options::value<uint>()->default_value(1024),
      "height of the output image"
    )("width,w",
      boost::program_options::value<uint>()->default_value(1024),
      "width of the output image"
    )("x",
      boost::program_options::value<float>(),
      "x-index of the vector field slice"
    )("y",
      boost::program_options::value<float>(),
      "y-index of the vector field slice"
    )("z",
      boost::program_options::value<float>(),
      "z-index of the vector field slice");

    boost::program_options::parsed_options parsed =
        boost::program_options::command_line_parser(argc, argv)
            .options(opts)
            .run();

    boost::program_options::store(parsed, vm);

    if (vm.count("help")) {
        std::cout << opts << std::endl;
        std::exit(0);
    }

    if ((vm.count("x") + vm.count("y") + vm.count("z")) != 1) {
        BOOST_LOG_TRIVIAL(fatal) << "Must specify exactly one of x, y, or z!";
        std::exit(0);
    }

    try {
        boost::program_options::notify(vm);
    } catch (boost::program_options::required_option & e) {
        BOOST_LOG_TRIVIAL(fatal) << e.what();
        std::exit(1);
    }
}

int main(int argc, char ** argv)
{
    using field_t1 = covfie::field<covfie::backend::affine<
        covfie::backend::linear<covfie::backend::strided<
            covfie::vector::ulong3,
            covfie::backend::array<covfie::vector::float3>>>>>;
    using field_t2 = covfie::field<covfie::backend::affine<
        covfie::backend::linear<covfie::backend::strided<
            covfie::vector::ulong3,
            covfie::backend::array<covfie::vector::float3>>>>>;

    boost::program_options::variables_map vm;
    parse_opts(argc, argv, vm);

    BOOST_LOG_TRIVIAL(info) << "Welcome to the covfie magnetic field renderer!";
    BOOST_LOG_TRIVIAL(info) << "Using magnetic field file \""
                            << vm["input"].as<std::string>() << "\"";
    BOOST_LOG_TRIVIAL(info) << "Starting read of input file...";

    std::ifstream ifs(vm["input"].as<std::string>(), std::ifstream::binary);

    if (!ifs.good()) {
        BOOST_LOG_TRIVIAL(fatal) << "Failed to open input file "
                                 << vm["input"].as<std::string>() << "!";
        std::exit(1);
    }

    field_t1 f(ifs);
    ifs.close();

    BOOST_LOG_TRIVIAL(info) << "Casting magnetic field into CPU array...";

    field_t2 nf(f);

    BOOST_LOG_TRIVIAL(info) << "Creating magnetic field view...";

    field_t2::view_t fv(nf);

    BOOST_LOG_TRIVIAL(info) << "Allocating memory for output image...";

    std::unique_ptr<char[]> img = std::make_unique<char[]>(
        vm["width"].as<uint>() * vm["height"].as<uint>()
    );

    BOOST_LOG_TRIVIAL(info) << "Rendering magnetic field strength to image...";

    std::chrono::high_resolution_clock::time_point t1 =
        std::chrono::high_resolution_clock::now();

    for (uint x = 0; x < vm["width"].as<uint>(); ++x) {
        for (uint y = 0; y < vm["height"].as<uint>(); ++y) {
            float fx = x / static_cast<float>(vm["width"].as<uint>());
            float fy = y / static_cast<float>(vm["height"].as<uint>());

            decltype(fv)::output_t p;

            if (vm.count("x")) {
                p = fv.at(
                    vm["x"].as<float>(),
                    fx * 20000.f - 10000.f,
                    fy * 30000.f - 15000.f
                );
            } else if (vm.count("y")) {
                p = fv.at(
                    fx * 20000.f - 10000.f,
                    vm["y"].as<float>(),
                    fy * 30000.f - 15000.f
                );
            } else {
                p = fv.at(
                    fx * 20000.f - 10000.f,
                    fy * 20000.f - 10000.f,
                    vm["z"].as<float>()
                );
            }

            img[vm["height"].as<uint>() * x + y] =
                static_cast<char>(std::lround(
                    255.f *
                    std::min(
                        std::sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]), 1.0f
                    )
                ));
        }
    }

    std::chrono::high_resolution_clock::time_point t2 =
        std::chrono::high_resolution_clock::now();

    BOOST_LOG_TRIVIAL(info
    ) << "Rendering took "
      << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()
      << "us." << std::endl;

    BOOST_LOG_TRIVIAL(info) << "Saving image to file \""
                            << vm["output"].as<std::string>() << "\"...";

    render_bitmap(
        img.get(),
        vm["width"].as<uint>(),
        vm["height"].as<uint>(),
        vm["output"].as<std::string>()
    );

    BOOST_LOG_TRIVIAL(info) << "Procedure complete, goodbye!";
}
