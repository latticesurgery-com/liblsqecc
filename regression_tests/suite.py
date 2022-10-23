#!/usr/bin/env python3

"""
This file just forwards to crystalmountain.py setting up the environment. See its usage for details
"""


import os.path
import sys

sys.path.append("../external/crystalmountain")

try:
    import crystalmountain
except ModuleNotFoundError:
    print("Make sure to run suite.py from the regression_tests directory", file=sys.stderr)
    exit(1)

PROJECT_DIR = os.path.abspath("..")

POSSIBLE_SLICER_LOCATIONS = [
    f"{PROJECT_DIR}/cmake-build-debug",
    f"{PROJECT_DIR}/cmake-build-release",
    f"{PROJECT_DIR}/build",
]

slicer_locations = list(filter(lambda location: os.path.exists(f"{location}/lsqecc_slicer"), POSSIBLE_SLICER_LOCATIONS))
if not slicer_locations:
    print("No slicer found", file=sys.stderr)
    exit(1)
slicer_location = slicer_locations[0]

print(f"Using lsqecc_slicer from {slicer_location}")

os.environ["PATH"] += f":{slicer_location}"

exit(crystalmountain.main().value)
