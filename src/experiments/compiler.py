import json

def generate_cpp_code(party_id, json_data):
    if party_id not in [0, 1]:
        raise ValueError("party_id must be either 0 or 1")

    # Template for the C++ code
    cpp_template = '''
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
#include "add.hpp"
#include "fssgate.hpp"
#include "mult.hpp"
#include "select.hpp"

int main() {{
    {initialization}
    {operations}

    return {last_output};
}}
    '''

    # Initialization code based on party_id
    initialization_code = (
        """
    int          port         = comm::kDefaultPort;
    std::string  host_address = comm::kDefaultAddress;
    int          party_id     = {party_id};

    comm::CommInfo               comm_info(party_id, port, host_address);
    tools::secret_sharing::Party party(comm_info);
""".format(party_id=party_id)
    )

    # Parse JSON to generate operations
    operations = []
    program = json_data.get("program", [])
    for step in program:
        name = step.get("name")
        function = step.get("function")
        args = ', '.join(step.get("args", []))
        operations.append(f"uint32_t {name} = {function}(party, {args});")

    # Join operations and get the last output variable
    operations_code = "\n    ".join(operations)
    last_output = program[-1]["name"] if program else "0"

    # Generate the final C++ code
    cpp_code = cpp_template.format(
        initialization=initialization_code,
        operations=operations_code,
        last_output=last_output
    )

    return cpp_code

def save_cpp_code(filename, cpp_code):
    with open(filename, 'w') as file:
        file.write(cpp_code)

if __name__ == "__main__":
    # Example JSON
    example_json = {
        "program": [
            {"name": "sum1", "function": "add", "args": ["a0", "b0"]},
            {"name": "sum2", "function": "add", "args": ["c0", "d0"]},
            {"name": "prod", "function": "mult", "args": ["sum1", "sum2"]}
        ]
    }
    party_id = 0
    cpp_code = generate_cpp_code(party_id, example_json)
    save_cpp_code("generated_code.cpp", cpp_code)

