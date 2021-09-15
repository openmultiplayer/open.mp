#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <cxxopts.hpp>
#include <sdk.hpp>
#include "core_impl.hpp"
#include "util.hpp"

int main(int argc, char** argv)
{
    cxxopts::Options options(argv[0], "The open.mp game server");

    options.add_options()
    (
        "h,help",
        "Print usage information"
    )
    (
        "write-config",
        "Generate default config.json"
    )
    (
        "script",
        "Script to run",
        cxxopts::value<std::string>()
    )
    ;
    options.parse_positional("script");
    options.show_positional_help();

    auto result = options.parse(argc, argv);

    if(result.count("help"))
    {
        // Print help text
        std::cout << options.help() << std::endl;
        return 0;
    }

    if(result.count("write-config"))
    {
        // Generate config
        return !Config::writeDefault();
    }

    SET_TICKER_RESOLUTION(1);

    Core* core = new Core();

    if(result.count("script"))
    {
        core->config.setString(
            "entry_file",
            result["script"].as<std::string>()
        );
    }

    core->run();
}
