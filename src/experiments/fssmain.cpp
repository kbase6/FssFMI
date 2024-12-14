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
#include "../tools/random_number_generator.hpp"
#include "../tools/secret_sharing.hpp"
#include "../tools/tools.hpp"
#include "../utils/logger.hpp"
#include "../utils/utils.hpp"
#include "fssgate.hpp"

namespace {

void DisplayHelp() {
    std::cout << "Usage:" << std::endl;
    std::cout << "    ./bin/fssmain <party_id> <exec_mode> [options]" << std::endl;
    std::cout << "\n<party_id> : Party id (0 or 1) is required" << std::endl;
    std::cout << "<exec_mode> : Execution mode (setup or eval) is required" << std::endl;
    std::cout << "\noptions:" << std::endl;
    std::cout << "    -p, --port <port_number> : Specify port number (default: 55555)" << std::endl;
    std::cout << "    -s, --server <server_address> : Specify server address (default: 127.0.0.1)" << std::endl;
    std::cout << "    -o, --output <output_file> : Specify output file name" << std::endl;
    std::cout << "    -h, --help : Display help message" << std::endl;
}

void GenerateRandomNumbers(std::vector<uint32_t> &vec, const uint32_t bitsize) {
    // Generate random vector
    for (size_t i = 0; i < vec.size(); i++) {
        vec[i] = utils::Mod(tools::rng::SecureRng::Rand64(), bitsize);
    }
}

}    // namespace

int main(int argc, char *argv[]) {
    // Default parameters
    int           port         = comm::kDefaultPort;
    std::string   host_address = comm::kDefaultAddress;
    int           party_id     = -1;
    std::string   exec_mode;
    std::string   output_file;
    utils::FileIo io(false, ".log");

    // Command-line options
    const char *const short_opts  = "p:s:o:h";
    const option      long_opts[] = {
        {"port", required_argument, nullptr, 'p'},
        {"server", required_argument, nullptr, 's'},
        {"output", required_argument, nullptr, 'o'},
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
                case 'o':
                    output_file = optarg;
                    break;
                case 'h':
                    DisplayHelp();
                    return EXIT_SUCCESS;
                default:
                    std::cerr << "Invalid option\n\n";
                    DisplayHelp();
                    return EXIT_FAILURE;
            }
        } catch (const std::invalid_argument &) {
            std::cerr << "Invalid argument for option -" << static_cast<char>(opt) << ": " << optarg << " is not a number.\n";
            return EXIT_FAILURE;
        } catch (const std::out_of_range &) {
            std::cerr << "Argument out of range for option -" << static_cast<char>(opt) << ": " << optarg << " is too large.\n";
            return EXIT_FAILURE;
        } catch (...) {
            std::cerr << "Unexpected error occurred while processing option -" << static_cast<char>(opt) << ".\n";
            return EXIT_FAILURE;
        }
    }

    // Validate positional arguments
    if (optind + 1 < argc) {
        try {
            party_id = std::stoi(argv[optind]);
            if (party_id != 0 && party_id != 1) {
                std::cerr << "Invalid party_id. It must be 0 or 1.\n";
                return EXIT_FAILURE;
            }
            exec_mode = argv[optind + 1];
            if (exec_mode != "setup" && exec_mode != "eval") {
                std::cerr << "Invalid exec_mode. It must be 'setup' or 'eval'.\n";
                return EXIT_FAILURE;
            }
        } catch (const std::invalid_argument &) {
            std::cerr << "Party ID must be a valid integer.\n";
            return EXIT_FAILURE;
        } catch (const std::out_of_range &) {
            std::cerr << "Party ID is out of range.\n";
            return EXIT_FAILURE;
        }
    } else {
        std::cerr << "Party ID and exec_mode are required. Use -h, --help for usage.\n";
        return EXIT_FAILURE;
    }

    // Output parsed parameters for confirmation
    std::cout << "Party ID: " << party_id << "\n"
              << "Execution Mode: " << exec_mode << "\n"
              << "Port: " << port << "\n"
              << "Server Address: " << host_address << "\n"
              << "Output File: " << (output_file.empty() ? "Not specified" : output_file) << "\n";
    // Placeholder for main logic
    std::cout << "Program execution starts here...\n\n";

    comm::CommInfo               comm_info(party_id, port, host_address);
    tools::secret_sharing::Party party(comm_info);

    uint32_t                                     bitsize = 10;
    tools::secret_sharing::AdditiveSecretSharing ss(bitsize);

    if (exec_mode == "setup") {
        // ################################
        // ########### Setup ##############
        // ################################
        fss::ZeroTestSetup(bitsize);
        fss::EqualitySetup(bitsize);
        std::vector<uint32_t> database(254);
        GenerateRandomNumbers(database, 1);
        fss::FMISearchSetup(bitsize, database);

    } else if (exec_mode == "eval") {
        // ################################
        // ######## Execution #############
        // ################################
        utils::Logger::InfoLog(LOCATION, "Executing Zero Test...");

        utils::Logger::InfoLog(LOCATION, "Input: x = 0");
        tools::secret_sharing::share_t x = ss.Share(0);
        uint32_t                       z_0{0}, z_1{0};
        if (party.GetId() == 0) {
            z_0 = fss::ZeroTest(party, x.first, bitsize);
        } else {
            z_1 = fss::ZeroTest(party, x.second, bitsize);
        }
        uint32_t z = ss.Reconst(party, z_0, z_1);    // 実際はユーザが復元する部分
        utils::PrintValidity("Zero Test", z, 1, false);

        utils::Logger::InfoLog(LOCATION, "Input: x = 123");
        x = ss.Share(123);
        if (party.GetId() == 0) {
            utils::Logger::InfoLog(LOCATION, "Party 0: x_0 = " + std::to_string(x.first));
            z_0 = fss::ZeroTest(party, x.first, bitsize);
        } else {
            utils::Logger::InfoLog(LOCATION, "Party 1: x_1 = " + std::to_string(x.second));
            z_1 = fss::ZeroTest(party, x.second, bitsize);
        }
        z = ss.Reconst(party, z_0, z_1);    // 実際はユーザが復元する部分
        utils::PrintValidity("Zero Test", z, 0, false);

        utils::Logger::InfoLog(LOCATION, "Executing Equality Test...");

        utils::Logger::InfoLog(LOCATION, "Input: x = 123, y = 123");
        x                                = ss.Share(123);
        tools::secret_sharing::share_t y = ss.Share(123);
        uint32_t                       e_0{0}, e_1{0};
        if (party.GetId() == 0) {
            e_0 = fss::Equality(party, x.first, y.first, bitsize);
        } else {
            e_1 = fss::Equality(party, x.second, y.second, bitsize);
        }
        uint32_t e = ss.Reconst(party, e_0, e_1);    // 実際はユーザが復元する部分
        utils::PrintValidity("Equality Test", e, 1, false);

        utils::Logger::InfoLog(LOCATION, "Input: x = 123, y = 456");
        x = ss.Share(123);
        y = ss.Share(456);
        if (party.GetId() == 0) {
            e_0 = fss::Equality(party, x.first, y.first, bitsize);
        } else {
            e_1 = fss::Equality(party, x.second, y.second, bitsize);
        }
        e = ss.Reconst(party, e_0, e_1);    // 実際はユーザが復元する部分
        utils::PrintValidity("Equality Test", e, 0, false);

        utils::Logger::InfoLog(LOCATION, "Executing FMI Search...");
        std::vector<uint32_t> q = {1, 0, 0, 1, 0, 1, 1, 0, 0, 1};    // * Expected Match Length: 10
        utils::Logger::InfoLog(LOCATION, "Query: " + utils::VectorToStr(q));

        tools::secret_sharing::shares_t q_sh = ss.Share(q);
        uint32_t                        m_0{0}, m_1{0};
        if (party.GetId() == 0) {
            m_0 = fss::FMISearch(party, q_sh.first, bitsize);
        } else {
            m_1 = fss::FMISearch(party, q_sh.second, bitsize);
        }
        uint32_t m = ss.Reconst(party, m_0, m_1);    // 実際はユーザが復元する部分
        utils::PrintValidity("FMI Search", m, 10, false);
    }
    utils::Logger::InfoLog(LOCATION, "Program execution ends here...\n");

    return 0;
}
