# Regression test suite for `lsqecc_slicer`

## Basic usage
1. Make sure to have initialized submodules (`git submodule init && git submodule update --remote`) and built in either
`build`,`cmake-build-debug` or `cmake-build-release` (or edit the top of `suite.py` to specify a particular location).
2. Enter the `regression_tests` directory and run `./suite.py`

The output will show if some tests are failing.

### Creating a new case

1. Create a shell script in `cases`, say `cases/cool_tests/my_tests.case.sh`. Note the ending in `.case.sh`. Can use 
   existing ones as examples.
2. Run `./suite.py -g cases/cool_tests/my_tests.case.sh`
3. Check `cases/cool_tests/my_tests.spec` to make sure the test is what you expect

### Updating a test

Say `./suite.py` shows that `cases/cool_tests/my_tests.case.sh` is failing.

Check `cases/cool_tests/my_tests.spec.new` to see the new output.
You can use `./suite.py -s cases/cool_tests/my_tests.case.sh` to print a diff.

If `cases/cool_tests/my_tests.spec.new` has the output you want, use `./suite.py -c cases/cool_tests/my_tests.case.sh`
to update the test.

### More advanced usage

The file `suite.py` is just a convenience wrapper for the [Crystal Mountain testing script](https://github.com/gwwatkin/crystalmountain).
The purpose of having a dedicated `suite.py` is that finds the `lsqecc_slicer` executable. 
All Crystal Mountain commands are valid `suite.py` commands, since they are just
forwarded over.