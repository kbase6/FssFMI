/**
 * @file comm_test.cpp
 * @author tomo-uchiyama@moegi.waseda.jp
 * @date 2024-01-05
 * @copyright Copyright (c) 2024
 * @brief Communication test implementation.
 */

#include "comm.hpp"

#include "../utils/logger.hpp"
#include "../utils/timer.hpp"
#include "../utils/utils.hpp"
#include "client.hpp"
#include "server.hpp"

namespace comm {
namespace test {

bool Test_StartComm(const CommInfo &comm_info, Server &p0, Client &p1, const bool debug);
bool Test_ValueComm(const CommInfo &comm_info, Server &p0, Client &p1, const bool debug);
bool Test_ArrayComm(const CommInfo &comm_info, Server &p0, Client &p1, const bool debug);
bool Test_VectorComm(const CommInfo &comm_info, Server &p0, Client &p1, const bool debug);
bool Test_CountTotalComm(const CommInfo &comm_info, Server &p0, Client &p1, const bool debug);

void Test_Comm(const CommInfo &comm_info, const uint32_t mode, bool debug) {
    std::vector<std::string> modes         = {"Comm unit tests", "Start communication", "Value communication", "Array communication", "Vector communication", "Count total communication"};
    uint32_t                 selected_mode = mode;
    if (selected_mode < 1 || selected_mode > modes.size()) {
        utils::OptionHelpMessage(LOCATION, modes);
        exit(EXIT_FAILURE);
    }

    // Create server and client objects based on the party ID.
    debug = (selected_mode == 1) ? false : debug;
    Server p0(comm_info.port_number, debug);
    Client p1(comm_info.host_address, comm_info.port_number, debug);
    utils::PrintText(utils::Logger::StrWithSep(modes[selected_mode - 1]));
    if (selected_mode == 1) {
        utils::PrintTestResult("Test_StartComm", Test_StartComm(comm_info, p0, p1, debug));
        utils::PrintTestResult("Test_ValueComm", Test_ValueComm(comm_info, p0, p1, debug));
        utils::PrintTestResult("Test_VectorComm", Test_VectorComm(comm_info, p0, p1, debug));
        utils::PrintTestResult("Test_ArrayComm", Test_ArrayComm(comm_info, p0, p1, debug));
        utils::PrintTestResult("Test_CountTotalComm", Test_CountTotalComm(comm_info, p0, p1, debug));
    } else if (selected_mode == 2) {
        utils::PrintTestResult("Test_StartComm", Test_StartComm(comm_info, p0, p1, debug));
    } else if (selected_mode == 3) {
        utils::PrintTestResult("Test_StartComm", Test_StartComm(comm_info, p0, p1, debug));
        utils::PrintTestResult("Test_ValueComm", Test_ValueComm(comm_info, p0, p1, debug));
    } else if (selected_mode == 4) {
        utils::PrintTestResult("Test_StartComm", Test_StartComm(comm_info, p0, p1, debug));
        utils::PrintTestResult("Test_ArrayComm", Test_ArrayComm(comm_info, p0, p1, debug));
    } else if (selected_mode == 5) {
        utils::PrintTestResult("Test_StartComm", Test_StartComm(comm_info, p0, p1, debug));
        utils::PrintTestResult("Test_VectorComm", Test_VectorComm(comm_info, p0, p1, debug));
    } else if (selected_mode == 6) {
        utils::PrintTestResult("Test_StartComm", Test_StartComm(comm_info, p0, p1, debug));
        utils::PrintTestResult("Test_ValueComm", Test_ValueComm(comm_info, p0, p1, debug));
        utils::PrintTestResult("Test_CountTotalComm", Test_CountTotalComm(comm_info, p0, p1, debug));
    }
    p0.CloseSocket();
    p1.CloseSocket();
    utils::PrintText(utils::kDash);
}

bool Test_StartComm(const CommInfo &comm_info, Server &p0, Client &p1, const bool debug) {
    bool result = true;
    if (comm_info.party_id == 0) {
        // Test start communication.
        utils::Logger::DebugLog(LOCATION, "Party ID: 0", debug);
        utils::Logger::DebugLog(LOCATION, "Host address: " + comm_info.host_address, debug);
        p0.Setup();
        p0.Start();
    } else {
        // Test start communication.
        utils::Logger::DebugLog(LOCATION, "Party ID: 1", debug);
        utils::Logger::DebugLog(LOCATION, "Host address: " + comm_info.host_address, debug);
        p1.Setup();
        p1.Start();
    }
    return result;
}

bool Test_ValueComm(const CommInfo &comm_info, Server &p0, Client &p1, const bool debug) {
    bool result = true;
    if (comm_info.party_id == 0) {
        // Test value communication.
        uint32_t x = 12345;
        p0.SendValue(x);
        p0.RecvValue(x);
        result &= (x == 12346);
    } else {
        // Test value communication.
        uint32_t r_x;
        p1.RecvValue(r_x);
        result &= (r_x == 12345);
        r_x += 1;
        p1.SendValue(r_x);
    }
    return result;
}

bool Test_ArrayComm(const CommInfo &comm_info, Server &p0, Client &p1, const bool debug) {
    bool result = true;
    if (comm_info.party_id == 0) {
        // Test array communication.
        std::array<uint32_t, 2> arr2{0, 1};
        std::array<uint32_t, 4> arr4{0, 1, 2, 3};

        p0.SendArray(arr2);
        p0.RecvArray(arr2);
        result &= (arr2[0] == 1) && (arr2[1] == 2);

        p0.SendArray(arr4);
        p0.RecvArray(arr4);
        result &= (arr4[0] == 1) && (arr4[1] == 2) && (arr4[2] == 3) && (arr4[3] == 4);
    } else {
        // Test array communication.
        std::array<uint32_t, 2> r_arr2;
        std::array<uint32_t, 4> r_arr4;
        p1.RecvArray(r_arr2);
        result &= (r_arr2[0] == 0) && (r_arr2[1] == 1);
        for (size_t i = 0; i < r_arr2.size(); i++) {
            r_arr2[i] += 1;
        }
        p1.SendArray(r_arr2);

        p1.RecvArray(r_arr4);
        result &= (r_arr4[0] == 0) && (r_arr4[1] == 1) && (r_arr4[2] == 2) && (r_arr4[3] == 3);
        for (size_t i = 0; i < r_arr4.size(); i++) {
            r_arr4[i] += 1;
        }
        p1.SendArray(r_arr4);
    }
    return result;
}

bool Test_VectorComm(const CommInfo &comm_info, Server &p0, Client &p1, const bool debug) {
    bool result = true;
    if (comm_info.party_id == 0) {
        // Test vector communication.
        std::vector<uint32_t> vec = utils::CreateSequence(0, 10);
        p0.SendVector(vec);
        p0.RecvVector(vec);
        for (size_t i = 0; i < vec.size(); i++) {
            result &= (vec[i] == i + 1);
        }
    } else {
        // Test vector communication.
        std::vector<uint32_t> r_vec;
        p1.RecvVector(r_vec);
        for (size_t i = 0; i < r_vec.size(); i++) {
            r_vec[i] += 1;
        }
        p1.SendVector(r_vec);
    }
    return result;
}

bool Test_CountTotalComm(const CommInfo &comm_info, Server &p0, Client &p1, const bool debug) {
    bool result = true;
    // Test count total communication.
    if (comm_info.party_id == 0) {
        uint32_t total_bytes = 0;
        total_bytes          = p0.GetTotalBytesSent();
        utils::Logger::DebugLog(LOCATION, "Total bytes sent: " + std::to_string(total_bytes), debug);
        result &= (total_bytes > 0);
    } else {
        uint32_t total_bytes = 0;
        total_bytes          = p1.GetTotalBytesSent();
        utils::Logger::DebugLog(LOCATION, "Total bytes sent: " + std::to_string(total_bytes), debug);
        result &= (total_bytes > 0);
    }
    return result;
}

}    // namespace test
}    // namespace comm
