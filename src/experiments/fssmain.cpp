/**
 * @file fssmain.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief FssFMI implementation.
 */

#include <functional>
#include <getopt.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../comm/comm.hpp"
#include "../fss-base/dcf/distributed_comparison_function.hpp"
#include "../fss-base/ddcf/dual_dcf.hpp"
#include "../fss-base/dpf/distributed_point_function.hpp"
#include "../fss-base/prg/prg.hpp"
#include "../fss-gate/comp/integer_comparison.hpp"
#include "../fss-gate/fm-index/fss_fmi.hpp"
#include "../fss-gate/internal/fsskey_io.hpp"
#include "../fss-gate/rank/fss_rank.hpp"
#include "../fss-gate/zt/zero_test_dpf.hpp"
#include "../tools/secret_sharing.hpp"
#include "../tools/tools.hpp"
#include "../utils/file_io.hpp"
#include "../utils/logger.hpp"
#include "../utils/utils.hpp"

namespace {

std::vector<std::string> test_names = {
    "fileio",
    "comm",
    "ss",
    "prg",
    "dpf",
    "dcf",
    "ddcf",
    "keyio",
    "comp",
    "rank",
    "zt",
    "fmi"};

std::map<std::string, std::function<void()>> test_func_map;

void SetupTestFuncMap(tools::secret_sharing::Party &party, const comm::CommInfo &comm_info, fss::TestInfo &test_info) {
    test_func_map[test_names[0]]  = [&]() { utils::test::Test_FileIo(test_info.mode, test_info.dbg_info.debug); };
    test_func_map[test_names[1]]  = [&]() { comm::test::Test_Comm(comm_info, test_info.mode, test_info.dbg_info.debug); };
    test_func_map[test_names[2]]  = [&]() { tools::test::Test_SecretSharing(comm_info, test_info.mode, test_info.dbg_info.debug); };
    test_func_map[test_names[3]]  = [&]() { fss::prg::test::Test_Prg(test_info); };
    test_func_map[test_names[4]]  = [&]() { fss::dpf::test::Test_Dpf(test_info); };
    test_func_map[test_names[5]]  = [&]() { fss::dcf::test::Test_Dcf(test_info); };
    test_func_map[test_names[6]]  = [&]() { fss::ddcf::test::Test_Ddcf(test_info); };
    test_func_map[test_names[7]]  = [&]() { fss::internal::test::Test_FssKeyIo(test_info); };
    test_func_map[test_names[8]]  = [&]() { fss::comp::test::Test_Comp(party, test_info); };
    test_func_map[test_names[9]]  = [&]() { fss::rank::test::Test_FssRank(party, test_info); };
    test_func_map[test_names[10]] = [&]() { fss::zt::test::Test_ZeroTest(party, test_info); };
    test_func_map[test_names[11]] = [&]() { fss::fmi::test::Test_FssFmi(party, test_info); };
}

std::vector<std::string> bench_names = {
    "dpf",
    "fmi"};

std::map<std::string, std::function<void()>> bench_func_map;

void SetupBenchFuncMap(tools::secret_sharing::Party &party, fss::BenchInfo &bench_info) {
    bench_func_map[bench_names[0]] = [&]() { fss::dpf::bench::Bench_Dpf(bench_info); };
    bench_func_map[bench_names[1]] = [&]() { fss::fmi::bench::Bench_FssFmi(party, bench_info); };
}

void DisplayHelp() {
    std::cout << "Usage:" << std::endl;
    std::cout << "    ./bin/fssmain <party_id> <mode> [options]" << std::endl;
    std::cout << "\n<party_id> : Party id (0 or 1) is required" << std::endl;
    std::cout << "\n<mode> : 'test' or 'bench' to specify execution mode" << std::endl;
    std::cout << "\noptions:" << std::endl;
    std::cout << "    -p, --port <port_number> : Specify port number (default: 55555)" << std::endl;
    std::cout << "    -s, --server <server_address> : Specify server address (default: 127.0.0.1)" << std::endl;
    std::cout << "    -n, --name <function_name> : Specify function to run" << std::endl;
    std::cout << "    -m, --mode <mode> : Specify function mode" << std::endl;
    std::cout << "    -o, --output <output_file> : Specify output file name" << std::endl;
    std::cout << "    -i, --iteration <iteration> : Specify iteration number" << std::endl;
    std::cout << "    -h, --help : Display help message" << std::endl;
}

}    // namespace

int main(int argc, char *argv[]) {
    int           port         = comm::kDefaultPort;
    std::string   host_address = comm::kDefaultAddress;
    int           party_id     = -1;
    std::string   exec_mode;
    std::string   function_name;
    int           function_mode = 0;
    std::string   output_file;
    int           iteration = 1;
    utils::FileIo io(false, ".log");

    const char *const short_opts  = "p:s:n:m:o:i:h";
    const option      long_opts[] = {
        {"port", required_argument, nullptr, 'p'},
        {"server", required_argument, nullptr, 's'},
        {"name", required_argument, nullptr, 'n'},
        {"mode", required_argument, nullptr, 'm'},
        {"output", required_argument, nullptr, 'o'},
        {"iteration", required_argument, nullptr, 'i'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, no_argument, nullptr, 0}};

    while (true) {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);
        if (opt == -1)
            break;

        try {
            switch (opt) {
                case 'p':
                    port = std::stoi(optarg);
                    break;
                case 's':
                    host_address = optarg;
                    break;
                case 'n':
                    function_name = optarg;
                    break;
                case 'm':
                    function_mode = std::stoi(optarg);
                    break;
                case 'o':
                    output_file = optarg;
                    break;
                case 'i':
                    iteration = std::stoi(optarg);
                    break;
                case 'h':
                    DisplayHelp();
                    return EXIT_SUCCESS;
                default:
                    std::cerr << "Invalid option\n\n";
                    DisplayHelp();
                    return EXIT_FAILURE;
            }
        } catch (const std::invalid_argument &e) {
            std::cerr << "Invalid argument for option -" << static_cast<char>(opt) << ": " << optarg << " is not a number.\n";
            return EXIT_FAILURE;
        } catch (const std::out_of_range &e) {
            std::cerr << "Argument out of range for option -" << static_cast<char>(opt) << ": " << optarg << " is too large.\n";
            return EXIT_FAILURE;
        } catch (...) {
            std::cerr << "Unexpected error occurred while processing option -" << static_cast<char>(opt) << ": " << optarg << ".\n";
            return EXIT_FAILURE;
        }
    }

    if (optind + 1 < argc) {
        party_id  = std::stoi(argv[optind]);
        exec_mode = argv[optind + 1];
        if (party_id != 0 && party_id != 1) {
            std::cerr << "Invalid party_id. It must be 0 or 1.\n";
            return 1;
        }
        if (exec_mode != "test" && exec_mode != "bench") {
            std::cerr << "Invalid mode. It must be 'test' or 'bench'.\n";
            return 1;
        }
    } else {
        std::cerr << "Party ID and mode are required. Use -h, --help for usage.\n";
        return 1;
    }

    comm::CommInfo               comm_info(party_id, port, host_address);
    tools::secret_sharing::Party party(comm_info);
    fss::DebugInfo               dbg_info;

    // ########################################
    // ##### DEBUG INFORMATION (SET TRUE) #####
#ifdef LOG_LEVEL_DEBUG
    dbg_info.dpf_debug = true;
    // dbg_info.dcf_debug    = true;
    // dbg_info.ddcf_debug   = true;
    // dbg_info.comp_debug   = true;
    // dbg_info.rank_debug   = true;
    // dbg_info.zt_debug = true;
    // dbg_info.fmi_debug    = true;
    dbg_info.debug = true;
#endif
    // ########################################

    // ########################################
    // ############ TEST FUNCTIONS ############
    if (exec_mode == "test") {
        utils::Logger::InfoLog(LOCATION, "Mode: Test");
        fss::TestInfo test_info;
        test_info.domain_size = utils::CreateSequence(8, 9);
        test_info.dbg_info    = dbg_info;
        test_info.mode        = function_mode;

        SetupTestFuncMap(party, comm_info, test_info);
        if (test_func_map.find(function_name) != test_func_map.end()) {
            test_func_map[function_name]();    // Run the function associated with the function_name key
            if (!output_file.empty()) {
                const std::string log_file = utils::GetCurrentDirectory() + "/log/test/" + output_file + std::to_string(party_id);
                utils::Logger::SaveLogsToFile(log_file, false);
                utils::Logger::InfoLog(LOCATION, "Log file saved: " + log_file + ".log");
            }
        } else {
            utils::Logger::FatalLog(LOCATION, "Invalid function name: '" + function_name + "'. Please set the correct function name from the list below by using -n (--name) option.");
            utils::Logger::InfoLog(LOCATION, "Available functions list: [" + utils::VectorToStr<std::string>(test_names, ", ") + "]");
            return EXIT_FAILURE;
        }
    }
    // ########################################

    if (exec_mode == "bench") {
        // ########################################
        // ############ BENCH FUNCTIONS ###########
        utils::Logger::InfoLog(LOCATION, "Mode: Benchmark");
        fss::BenchInfo bench_info;
        bench_info.text_size = utils::CreateSequence(13, 28);
        bench_info.text_size = {28};
        // bench_info.query_size = utils::CreateSequence(3, 11);
        bench_info.query_size     = {4};
        bench_info.mode           = function_mode;
        bench_info.experiment_num = iteration;
        bench_info.limit_time_ms  = 1800000;    // 30 min
        bench_info.dbg_info       = dbg_info;

        SetupBenchFuncMap(party, bench_info);
        if (bench_func_map.find(function_name) != bench_func_map.end()) {
            bench_func_map[function_name]();    // Run the function associated with the function_name key
            if (!output_file.empty()) {
                std::string log_file = utils::GetCurrentDirectory() + "/log/bench/" + output_file + "_P" + std::to_string(party_id);
                io.ClearFileContents(log_file);
                utils::Logger::SaveLogsToFile(log_file, false);
            } else {
                std::string log_file = utils::GetCurrentDirectory() + "/log/bench/" + function_name + "/test" + "_P" + std::to_string(party_id);
                io.ClearFileContents(log_file);
                utils::Logger::SaveLogsToFile(log_file, false);
            }
        } else {
            utils::Logger::FatalLog(LOCATION, "Invalid function name: '" + function_name + "'. Please set the correct function name from the list below by using -n (--name) option.");
            utils::Logger::InfoLog(LOCATION, "Available functions list: [" + utils::VectorToStr<std::string>(bench_names, ", ") + "]");
            return EXIT_FAILURE;
        }

        // ########################################
    }

    // TODO: Apply full domain evaluation and early termination to DCF and DDCF

    return 0;
}
