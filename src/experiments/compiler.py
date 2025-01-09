import json
import sys
import subprocess
import os

def parse_json(json_str):
    """
    Parse the JSON string into a Python dictionary.
    """
    try:
        data = json.loads(json_str)
        return data
    except json.JSONDecodeError as e:
        print("Invalid JSON format:", e)
        return None

def is_numeric(value):
    """
    Check if 'value' is numeric (int/float) or can be cast to float.
    """
    if isinstance(value, (int, float)):
        return True
    try:
        float(value)
        return True
    except (ValueError, TypeError):
        return False

def generate_value(val):
    """
    Convert a JSON 'value' (number or string) into a C++ expression.
    """
    if is_numeric(val):
        return str(val)
    else:
        return val  # treat as a variable name

def generate_condition(cond):
    """
    cond example: { "lhs": "func2", "op": ">", "rhs": 5 }
    Return e.g. "func2 > 5".
    """
    lhs = generate_value(cond["lhs"])
    op = cond["op"]
    rhs = generate_value(cond["rhs"])
    return f"{lhs} {op} {rhs}"

def indent_code(code, level=0):
    """
    Indent each line in 'code' by 'level' spaces.
    """
    indentation = " " * level
    lines = code.split("\n")
    return "\n".join(indentation + line for line in lines if line.strip() != "")

def generate_block(instructions, indent_level=4, final_var=None):
    """
    Generate a code block from a list of instructions.

    :param instructions: list of instruction dicts
    :param indent_level: indentation in spaces
    :param final_var:
       - If None, we do NOT force the block to produce a single final variable.
       - If set to a string (e.g. "func3"), then the LAST instruction in this block
         must assign to 'final_var'.
         If the last instruction is itself an if/for, we recursively force that structure
         to produce final_var.
    :return: (code_string, last_var)
       - code_string: The generated C++ code lines
       - last_var: The name of the last assigned variable in this block (might be final_var, or None if none assigned)
    """
    code_lines = []
    last_assigned = None

    for i, instr in enumerate(instructions):
        is_last = (i == len(instructions) - 1)
        
        # If we need to produce final_var in this block AND this is the last instruction:
        # we pass final_var down so the instruction is forced to assign to it.
        forced_var = final_var if (is_last and final_var) else None
        
        sub_code, sub_assigned = generate_instruction(instr, indent_level, forced_var)
        code_lines.append(sub_code)
        
        if sub_assigned:
            last_assigned = sub_assigned
    
    # If the block as a whole was asked to produce final_var,
    # but the last instruction didn't assign anything, we have an issue.
    # Potentially we can set final_var = 0, or just do nothing.
    if final_var and (last_assigned != final_var):
        # Fallback: if we never assigned final_var, we'll do a default assignment
        code_lines.append(f"{final_var} = 0;  // fallback if never assigned")

    block_code = "\n".join(code_lines)
    return (block_code, last_assigned)

def generate_instruction(instr, indent_level, forced_var):
    """
    Generate code for a single instruction.
    :param instr: dict (e.g. { "type": "call", "name": "...", ... })
    :param indent_level: indentation
    :param forced_var: if not None, we must produce forced_var as the final assigned variable

    :return: (code_string, last_assigned_var_or_none)
    """
    itype = instr["type"]

    # 1) "assign"
    if itype == "assign":
        var_name = instr["name"]
        value = generate_value(instr["value"])
        if forced_var:
            # If forced_var is set, we ignore var_name and directly do forced_var = ...
            code = f"{forced_var} = {value};"
            return (indent_code(code, indent_level), forced_var)
        else:
            code = f"uint32_t {var_name} = {value};"
            return (indent_code(code, indent_level), var_name)

    # 2) "call"
    elif itype == "call":
        # e.g. "name": "func1", "function": "add", "args": ["func2", 2]
        var_name = instr.get("name")  # might be None
        func_name = instr["function"]
        args = instr.get("args", [])
        arg_exprs = [generate_value(a) for a in args]
        joined_args = ", ".join(arg_exprs)
        
        # If forced_var is set, we ignore var_name and directly do forced_var = function(...)
        if forced_var:
            code = f"{forced_var} = {func_name}({joined_args});"
            return (indent_code(code, indent_level), forced_var)
        else:
            if var_name:
                code = f"uint32_t {var_name} = {func_name}({joined_args});"
                return (indent_code(code, indent_level), var_name)
            else:
                # Just a function call with no assigned variable
                code = f"{func_name}({joined_args});"
                return (indent_code(code, indent_level), None)

    # 3) "if"
    elif itype == "if":
        # e.g. "name": "func3", "condition": {...}, "then": [...], "else": [...]
        if_var = instr.get("name")  # The final variable that the if must produce
        condition_str = generate_condition(instr["condition"])
        then_block = instr.get("then", [])
        else_block = instr.get("else", [])

        # If we have forced_var from parent (e.g. parent's block says "assign to X"),
        # but the if also has "name", we give priority to the if's own name.
        # Because that’s presumably what the user wants from this if statement.
        # Then afterwards, if the parent's forced_var is also set, we do "forced_var = if_var".
        local_if_var = if_var if if_var else forced_var

        code_list = []
        # Declare local_if_var if it's not None
        if local_if_var:
            code_list.append(f"uint32_t {local_if_var};")

        # Generate the 'then' block, forcing it to produce local_if_var
        code_list.append(f"if ({condition_str}) {{")
        then_code, then_assigned = generate_block(
            then_block, 
            indent_level,
            final_var=local_if_var  # Force the then-block to produce local_if_var
        )
        code_list.append(then_code)
        code_list.append("}")

        # Generate the 'else' block, also forcing it to produce local_if_var
        if else_block:
            code_list.append("else {")
            else_code, else_assigned = generate_block(
                else_block,
                indent_level,
                final_var=local_if_var
            )
            code_list.append(else_code)
            code_list.append("}")

        joined_code = "\n".join(code_list)
        
        # If the `if` node had "name", that variable is the final of this instruction.
        # Otherwise, if we used parent's forced_var, that is the final.
        final_name = if_var if if_var else forced_var

        # If the parent's forced_var is different from if_var,
        # we must do forced_var = if_var after the if statement:
        post_assign_code = ""
        if forced_var and (if_var and forced_var != if_var):
            # "forced_var = if_var;"
            post_assign_code = f"{forced_var} = {if_var};"

        # Combine everything
        full_code = indent_code(joined_code, indent_level)
        if post_assign_code:
            full_code += "\n" + indent_code(post_assign_code, indent_level)

        return (full_code, final_name)

    # 4) "for"
    elif itype == "for":
        # e.g. "init":{"name":"i","value":0}, "condition":..., "update":"i++", "body":[...]
        init_obj = instr["init"]
        cond_obj = instr["condition"]
        update_str = instr["update"]
        body = instr.get("body", [])

        init_str = f"uint32_t {init_obj['name']} = {generate_value(init_obj['value'])};"
        cond_str = generate_condition(cond_obj)

        code_list = []
        code_list.append(f"for ({init_str} {cond_str}; {update_str}) {{")

        # If we must produce forced_var from within the loop,
        # then every iteration’s last instruction sets forced_var.
        body_code, body_assigned = generate_block(body, indent_level, final_var=forced_var)
        code_list.append(body_code)
        code_list.append("}")

        joined_code = "\n".join(code_list)
        return (indent_code(joined_code, indent_level), forced_var if forced_var else body_assigned)

    else:
        # Unknown instruction
        code = f"// Unknown instruction type: {itype}"
        return (indent_code(code, indent_level), None)

def generate_cpp_prefix():
    """
    Generate #includes and any other top-level code.
    """
    includes = [
        '<iostream>',
        '<functional>',
        '<map>',
        '<string>',
        '<vector>',
        '"../comm/comm.hpp"',
        '"../tools/random_number_generator.hpp"',
        '"../tools/secret_sharing.hpp"',
        '"../tools/tools.hpp"',
        '"../utils/logger.hpp"',
        '"../utils/utils.hpp"',
        '"add.hpp"',
        '"mult.hpp"',
        '"fssgate.hpp"'
    ]
    prefix = ""
    for inc in includes:
        prefix += f"#include {inc}\n"
    prefix += "\n"
    return prefix

def find_final_variable(program):
    """
    Scan from the end of the top-level instructions to find the last
    'assign' or 'call' that has a 'name'. If there's an 'if' or 'for',
    it won't produce a direct name unless the user gave 'if': {"name":...}.
    But we won't dive deeper here.
    """
    for instr in reversed(program):
        t = instr["type"]
        if t in ("assign", "call") and "name" in instr:
            return instr["name"]
        if t == "if" and "name" in instr:
            return instr["name"]
    return None

def generate_cpp_main(program):
    """
    Build the main() function from the top-level 'program'.
    We end by returning the last variable we find (if any).
    """
    cpp_main = "int main() {\n"
    # We'll generate the block normally
    body_code, last_assigned = generate_block(program, indent_level=4)
    cpp_main += body_code + "\n"

    # Look for a final variable if not found in last_assigned
    final_var = last_assigned if last_assigned else find_final_variable(program)
    if final_var:
        cpp_main += f"    return {final_var};\n"
    else:
        cpp_main += "    return 0;\n"

    cpp_main += "}\n"
    return cpp_main

def generate_code(json_data):
    """
    Generate the full .cpp code from the JSON structure.
    """
    program = json_data.get("program", [])
    prefix = generate_cpp_prefix()
    main_code = generate_cpp_main(program)
    return prefix + main_code

def compile_code(source_file, output_file):
    """
    Optional: compile the generated .cpp.
    """
    if not os.path.exists(source_file):
        sys.exit(1)

    compile_cmd = ["g++", "-O2", source_file, "-o", output_file]
    compile_proc = subprocess.run(compile_cmd, capture_output=True, text=True)
    if compile_proc.returncode != 0:
        print(compile_proc.stderr)
        sys.exit(1)

    run_proc = subprocess.run([f"./{output_file}"], capture_output=True, text=True)
    if run_proc.returncode != 0:
        print(run_proc.stderr)
    else:
        print(run_proc.stdout)

def compiler(json_input):
    """
    Main entry point: parse JSON, generate .cpp code, write to file, 
    optionally compile and run.
    """
    data = parse_json(json_input)
    if not data:
        return

    cpp_code = generate_code(data)
    with open("generated_code.cpp", "w") as f:
        f.write(cpp_code)

    # Optionally compile and run:
    # compile_code("generated_code.cpp", "generated_executable")

if __name__ == "__main__":
    # Example nested JSON (be sure to fix the final assignment if you want a valid final var).
    example_json = r"""
{
  "program": [
    {
      "type": "assign",
      "name": "product",
      "value": "mult(a, x)"
    },
    {
      "type": "assign",
      "name": "sum",
      "value": "add(c, z)"
    },
    {
      "type": "call",
      "name": "comparisonResult",
      "function": "Compare",
      "args": ["product", "sum"]
    }
  ]
}
    """
    compiler(example_json)
