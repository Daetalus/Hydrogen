
#
#  Runtime Tests
#

import os
import sys
import platform

from os.path import join, dirname, basename, isdir, splitext, realpath
from subprocess import Popen, PIPE
from threading import Timer


# The path to the command line interface which will execute the Hydrogen code
# for us.
root_directory = dirname(dirname(dirname(realpath(__file__))))
cli_path = join(join(root_directory, "build"), "cli")

# The amount of time in seconds to let a test case run before killing it.
timeout = 2


# Available color codes
COLOR_NONE    = "\x1B[0m"
COLOR_RED     = "\x1B[31m"
COLOR_GREEN   = "\x1B[32m"
COLOR_YELLOW  = "\x1B[33m"
COLOR_BLUE    = "\x1B[34m"
COLOR_MAGENTA = "\x1B[35m"
COLOR_CYAN    = "\x1B[36m"
COLOR_WHITE   = "\x1B[37m"
COLOR_BOLD    = "\x1B[1m"


# Prints some color text to the standard output.
def print_color(color):
    if platform.system() != "Windows":
        sys.stdout.write(color)


# Extracts the expected output of a test case from its source code. Returns
# a list of strings, one for each line of expected output.
def expected_output(source):
    # We need to find every instance of `// expect: ` and concatenate them
    # together, separated by newlines
    key = "// expect: "
    result = []
    line_number = 1
    for line in source.splitlines():
        # Find what we're searching for
        found = line.find(key)
        if found == -1:
            continue

        # Append the rest of the line
        index = found + len(key)
        result.append({"content": line[index:], "line": line_number})
        line_number += 1
    return result


# Runs a test program from its path. Returns the exit code for the process and
# what was written to the standard output. Returns -1 for the error code if the
# process timed out.
def run_test(path):
    # Create the process
    proc = Popen([cli_path, path], stdin=PIPE, stdout=PIPE, stderr=PIPE)

    # Kill test cases that take longer than 2 seconds
    timed_out = [False]
    def kill(test_case):
        timed_out[0] = True
        test_case.kill()
    timer = Timer(2, kill, [proc])

    # Execute the test case
    exit_code = -1
    output = None
    try:
        timer.start()
        output, error = proc.communicate()
        if not timed_out[0]:
            exit_code = proc.returncode
    finally:
        timer.cancel()

    return (output, exit_code)


# Prints an error message to the standard output.
def print_error(message):
    print_color(COLOR_RED + COLOR_BOLD)
    sys.stdout.write("[Error] ")
    print_color(COLOR_NONE)
    print(message)


# Validates the output of a test case.
def validate(path, expected, output, exit_code):
    # Parse the output into lines
    try:
        output = output.decode("utf-8").replace("\r\n", "\n")
    except:
        print_error("Failed to decode output")
        return False

    # Check if the test case timed out
    if exit_code == -1:
        print_error("Timed out")
        return False

    # Check if the test case returned an error
    if exit_code != 0:
        print_error("Compilation or runtime failure")
        print(output)
        return False

    # Convert output into multiple lines
    output_lines = []
    if len(output) > 0:
        output_lines = output.strip().split("\n")

    # Check output lengths match
    if len(expected) != len(output_lines):
        print_error("Incorrect number of output lines")
        return False

    # Check each line
    for i in range(len(output_lines)):
        expected_line = expected[i].content
        if output_lines[i] != expected_line:
            line_number = expected[i].line_number
            print_error("Incorrect output on line " + str(line_number) +
                    ": expected " + expected_line + ", got " + output_lines[i])
            return False

    # Passed test
    print_color(COLOR_BOLD + COLOR_GREEN)
    print("[Passed]")
    print_color(COLOR_NONE)
    return True


# Executes the runtime test for the Hydrogen code at `path`.
def test(path):
    # Print info
    print_color(COLOR_BLUE + COLOR_BOLD)
    sys.stdout.write("[Test] ")
    print_color(COLOR_NONE)

    suite = basename(dirname(path))
    name = splitext(basename(path))[0].replace("_", " ")
    print("Testing " + suite + " => " + name)

    # Open the input file
    input_file = open(path, "r")
    if not input_file:
        print_error("Failed to open file")
        return False

    # Read the contents of the file
    source = input_file.read()
    input_file.close()

    # Extract the expected output for the case
    expected = expected_output(source)

    # Get the output and exit code for the test case
    output, exit_code = run_test(path)

    # Validates a test case's output
    return validate(path, expected, output, exit_code)


# Tests all Hydrogen files in a directory. Returns the total number of tests,
# and the number of tests passed
def test_dir(path):
    total = 0
    passed = 0
    files = os.listdir(path)
    for case in files:
        case_path = join(path, case)
        if isdir(case_path):
            local_total, local_passed = test_dir(case_path)
            total += local_total
            passed += local_passed
        elif splitext(case_path)[1] == ".hy":
            if test(case_path):
                passed += 1
            total += 1
    return (total, passed)


# Test all files in this directory
total, passed = test_dir(sys.argv[1])

# Print final message
print("")
print_color(COLOR_BOLD)
if total == passed:
    # All tests passed
    print_color(COLOR_GREEN)
    sys.stdout.write("[Success] ")
    print_color(COLOR_NONE)
    print("All tests passed!")
else:
    # Not all tests passed
    print_color(COLOR_RED)
    sys.stdout.write("[Failure] ")
    print_color(COLOR_NONE)
    print(str(passed) + " of " + str(total) + " tests passed")
    sys.exit(1)
