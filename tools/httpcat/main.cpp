//
// Copyright (c) 2020 Richard Hodges (hodges.r@gmail.com)
// Copyright (c) 2021 Sergey Ilinykh (rion4ik@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "http/connection_cache.hpp"

#include <boost/program_options.hpp>

#include <cstdio>
#include <iostream>

net::awaitable< void >
visit_site(http::connection_cache &cache, std::string const &url, bool verbose = false)
{
    auto then = std::chrono::steady_clock::now();

    try
    {
        auto result = co_await cache.call(http::verb::get, url);
        auto time   = std::chrono::duration_cast< std::chrono::milliseconds >(
            std::chrono::steady_clock::now() - then);
        if (verbose)
        {
            std::cout << "GET " << url << " -> " << result->base().result_int()
                      << " " << result->base().reason() << " in "
                      << (time).count() << "ms\n";
        }
        std::cout << result->body();
    }
    catch (std::exception &e)
    {
        auto time = std::chrono::duration_cast< std::chrono::milliseconds >(
            std::chrono::steady_clock::now() - then);
        if (verbose)
        {
            std::cerr << "GET " << url << " -> "
                      << " exception: " << e.what() << " in " << (time).count()
                      << "ms\n";
        }
    }
}

int
main(int argc, char **argv)
{
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
        ("help,h", "produce a help message")
        ("verbose,v", "verbose output")
        ("url", po::value<std::vector<std::string>>(), "input file");

    po::positional_options_description p;
    p.add("url", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    if (!vm.count("url")) {
        std::cerr << "No input file";
        return -1;
    }
    auto urls = vm["url"].as<std::vector<std::string>>();
    bool verbose = vm.count("verbose");

    net::io_context ioctx;

    http::connection_cache cache(ioctx.get_executor());
    for (auto const &url: urls) {
        net::co_spawn(
            ioctx.get_executor(), visit_site(cache, url, verbose), net::detached);
    }

    ioctx.run();
}
