import subprocess
import sys
import argparse
import time
import threading


def run_standalone_program(party_id, mode, func_mode, port=None, server=None, name=None, output=None, iteration=None):
    cmd = ["./bin/fssmain", str(party_id), mode, "-m", str(func_mode)]

    if port:
        cmd.extend(["-p", str(port)])
    if server:
        cmd.extend(["-s", server])
    if name:
        cmd.extend(["-n", name])
    if output:
        cmd.extend(["-o", output])
    if iteration:
        cmd.extend(["-i", str(iteration)])

    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    # Real-time output
    for line in process.stdout:
        print(line, end="")
    for line in process.stderr:
        print(line, end="", file=sys.stderr)

    process.stdout.close()
    process.stderr.close()
    process.wait()


def run_server(party_id, mode, name, func_mode, port=None, server=None):
    cmd = ["./bin/fssmain", str(party_id), mode, "-n", name, "-m", str(func_mode)]

    if port:
        cmd.extend(["-p", str(port)])
    if server:
        cmd.extend(["-s", server])

    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    # Real-time output for server
    def print_output(stream):
        for line in iter(stream.readline, ''):
            print(line, end="")

    stdout_thread = threading.Thread(target=print_output, args=(process.stdout,))
    stderr_thread = threading.Thread(target=print_output, args=(process.stderr,))
    stdout_thread.start()
    stderr_thread.start()

    return process, stdout_thread, stderr_thread


def run_client(party_id, mode, name, func_mode, port=None, server=None, show_output=False):
    cmd = ["./bin/fssmain", str(party_id), mode, "-n", name, "-m", str(func_mode)]

    if port:
        cmd.extend(["-p", str(port)])
    if server:
        cmd.extend(["-s", server])

    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    if show_output:
        # Real-time output for client
        for line in process.stdout:
            print("[C]" + line, end="")
        for line in process.stderr:
            print("[C]" + line, end="", file=sys.stderr)

        process.stdout.close()
        process.stderr.close()
        process.wait()
    else:
        process.wait()


def run_server_client_mode(name, func_mode, port=None, server=None, show_client_output=False):
    server_process, stdout_thread, stderr_thread = run_server(0, "test", name, func_mode, port=port, server=server)
    time.sleep(1)  # Give server time to start up
    run_client(1, "test", name, func_mode, port=port, server=server, show_output=show_client_output)

    server_process.terminate()
    stdout_thread.join()
    stderr_thread.join()
    server_process.wait()
    # print(f"Server process for '{name}' with func_mode = {func_mode} terminated.")


def main():
    parser = argparse.ArgumentParser(description='Run fssmain with specific options.')
    parser.add_argument('--port', type=int, help='Port number')
    parser.add_argument('--server', type=str, help='Server address')
    parser.add_argument('--show_client_output', action='store_true', help='Show client output')
    parser.add_argument('-u', '--unit_test', action='store_true', help='Run unit test')

    args = parser.parse_args()

    if not any(vars(args).values()):
        parser.print_help()
        return

    if args.unit_test:
        run_standalone_program(0, "test", 1, port=args.port, server=args.server, name="fileio")
        run_server_client_mode("comm", 1, port=args.port, server=args.server, show_client_output=args.show_client_output)
        run_server_client_mode("ss", 1, port=args.port, server=args.server, show_client_output=args.show_client_output)
        for name in ["prg", "dpf", "dcf", "ddcf"]:
            run_standalone_program(0, "test", 1, port=args.port, server=args.server, name=name)
        for name in ["keyio", "rank", "zt", "fmi"]:
            if name == "keyio":
                run_standalone_program(0, "test", 1, port=args.port, server=args.server, name=name)
                continue
            run_server_client_mode(name, 1, port=args.port, server=args.server, show_client_output=args.show_client_output)


if __name__ == "__main__":
    main()
