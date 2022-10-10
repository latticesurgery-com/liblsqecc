#!/usr/bin/python

import argparse
import os
import re
import shutil
from glob import glob

import subprocess


class Prettify:
    @staticmethod
    def cyan(s: str):
        return "\033[36m"+s+"\033[0m"
    @staticmethod
    def magenta(s: str):
        return "\033[35m"+s+"\033[0m"
    @staticmethod
    def light_magenta(s: str):
        return "\033[95m"+s+"\033[0m"
    @staticmethod
    def yellow(s: str):
        return "\033[33m"+s+"\033[0m"
    @staticmethod
    def blue(s: str):
        return "\033[34m"+s+"\033[0m"
    @staticmethod
    def red(s: str):
        return "\033[31m"+s+"\033[0m"
    @staticmethod
    def green(s: str):
        return "\033[32m"+s+"\033[0m"


def write_file(file, contents):
    with open(file, 'w') as f:
        f.write(contents)

def read_file(file):
    with open(file, 'r') as f:
        return f.read()

def clear_console_line():
    print("\r" + " "*shutil.get_terminal_size((80,20)).columns + "\r", end="")


def run_case_file(case):
    return subprocess.check_output(f"source {case}", shell=True).decode('utf-8')

k_test_statuses_shorthands = {
    'passing' : Prettify.green('[P]'),
    'failing' : Prettify.red('[F]'),
    'nospec' : Prettify.yellow('[N]')
}


def handle_case(args, case):
    if not os.path.exists(case):
        print(Prettify.red('[CASE NOT FOUND]'), case)
        return

    spec = re.sub(r".case.sh$",".spec", case)
    new_spec = re.sub(r".case.sh$",".spec.new", case)

    def determine_status():
        if os.path.exists(new_spec):
            return 'failing'
        elif os.path.exists(spec):
            return 'passing'
        return 'nospec'

    if args.failing and determine_status() != 'failing':
        return
    if args.passing and determine_status() != 'passing':
        return
    if args.nospec and determine_status() != 'nospec':
        return

    if args.list:
        print(f"{k_test_statuses_shorthands[determine_status()]} {case}")
        return

    if args.removeoutput:
        try:
            os.remove(spec)
            print(Prettify.magenta('[REMOVED]'), spec)
        except OSError:
            print(Prettify.yellow('[NOT FOUND]'), spec)
    elif args.copyover:
        if os.path.exists(new_spec):
            os.replace(new_spec, spec)
            print(Prettify.green('[COPIED OVER]'), case)
        else:
            print(Prettify.yellow('[NOTHING TO COPY OVER]'), case)
    elif args.showdiffs:
        if os.path.exists(new_spec):
            os.system(f'diff -u --color {spec} {new_spec}')
        else:
            if not args.quiet:
                print(Prettify.green('[NO DIFF]'), case)


    else: # Things that actually run the tests
        def run_with_console_update():
            print(Prettify.cyan('[RUNNING]'), case, end="", flush=True)
            result = run_case_file(case)
            clear_console_line()
            return result

        if args.displayonly:
            print(Prettify.cyan('[RUN]'), case)
            print(run_with_console_update())
            print(Prettify.cyan('[END OF OUTPUT]'), '(no action performed)')
            return

        if args.generate:
            if not os.path.exists(spec):
                write_file(spec, run_with_console_update())
                print(Prettify.blue('[GENERATED SPEC]'), spec)
            else:
                print(Prettify.yellow('[SPEC ALREADY EXISTS]'), spec)
        else:
            # regular test run
            if not os.path.exists(spec):
                print(Prettify.yellow('[SPEC DOES NOT EXIST]'), spec)
                return

            result = run_with_console_update()
            if read_file(spec) == result:
                if not args.quiet:
                    print(Prettify.green('[PASSED]'), case)
            else:
                write_file(new_spec, result)
                print(Prettify.red('[FAILED]'), case)



if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Run a regression test suite for liblsqecc')
    parser.add_argument('cases',
                        metavar="CASE",
                        help="Specify one or more particular test cases",
                        nargs='*')


    filter_group = parser.add_mutually_exclusive_group()
    filter_group.add_argument('-f','--failing',
                        help="Only consider cases that failed in the latest run",
                        action='store_true')
    filter_group.add_argument('-n','--nospec',
                        help="Only consider cases that don't have a spec defined",
                        action='store_true')
    filter_group.add_argument('-a', '--passing',
                        help="Only consider cases that passed in the last run",
                        action='store_true')

    action_group = parser.add_mutually_exclusive_group()
    action_group.add_argument('-p','--displayonly',
                        help='display the results of the execution without writing anything',
                        action='store_true')
    action_group.add_argument('-c','--copyover',
                        help='Instead of running, copy over the results from the previous run',
                        action='store_true')
    action_group.add_argument('-s','--showdiffs',
                        help='Show diffs after failed tests',
                        action='store_true')
    action_group.add_argument('-r','--removeoutput',
                        help='Removes outputs for this test',
                        action='store_true')
    action_group.add_argument('-g','--generate',
                        help='Generates tests if they don\'t exist yet, otherwise do nothing',
                        action='store_true')
    action_group.add_argument('-l','--list',
                        action='store_true',
                        help='List available tests')

    parser.add_argument('-q','--quiet',
                        help='Don\'t show passing tests',
                        action='store_true')

    args = parser.parse_args()

    cases = None
    if args.cases:
        cases = args.cases
    else:
        cases = glob('cases/**/*.case.sh', recursive=True)

    for case in cases:
        handle_case(args, case)