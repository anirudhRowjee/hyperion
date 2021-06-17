# testing script for Hyperion
from typing import List, Tuple
import unittest
from subprocess import run, PIPE

DATABASE_RAW_COMMAND = "./hyperion"


def get_commands_from_array(command_array):
    # takes an array of commands, formats them in the input
    # format for the binary to accept, and runs them
    return "\n".join([str(x) for x in command_array]) + "\n"


def decompose_output_from_program(output_string):
    # takes the STDOUT from the binary and returns the array of commands
    return [str(x) for x in output_string.split("\n")]


def run_test_commands(commands):
    # takes the string of commands, runs them all and returns
    # stdout
    output = run([DATABASE_RAW_COMMAND], stdout=PIPE, input=commands, encoding="ascii")
    return (output.returncode, output.stdout)


def validate_test(command_list, target_output_list):
    # returns a boolean with the status of the commands to be run
    return_code, stdout = run_test_commands(get_commands_from_array(command_list))
    print(stdout, return_code)
    if return_code != 0:
        print(f"Failed with {return_code}")
        return False
    else:
        print("input array")
        print(command_list)
        output_list = decompose_output_from_program(stdout)
        print("output array")
        print(output_list)
        return output_list == target_output_list


class BasicTest(unittest.TestCase):
    # sanity checks
    def test_basic_prompt(self):
        self.assertTrue(validate_test([".exit"], ["H > "]))


class QueryTest(unittest.TestCase):
    def test_single_row_insert(self):
        # runs tests for basic queries
        self.assertTrue(
            validate_test(
                ["insert 1 A abc@amail.com", "select", ".exit"],
                [
                    "H > Executed",
                    "H > (1, A, abc@amail.com)",
                    "Executed",
                    "H > ",
                ],
            )
        )

    def test_insertion_error(self):
        # expect the DB to fail when you insert too many rows
        queryset = [f"insert {x} user{x} user{x}@x.com" for x in range(1, 1402)]
        queryset.append(".exit")
        self.assertFalse(validate_test(queryset, []))


if __name__ == "__main__":
    unittest.main()
