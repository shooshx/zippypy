libPyVM is a compact Python 2.7 interpreter that can be used in various places where the standard CPython interpreter is not appropriate or cumbersome to use.

- The entire interpreter is fully instantiatable. This means you can have multiple interpreters running in a single process. There is no global state or variables.
- Very light weight. The entire interpreter is about 6000 lines of code, excluding tests and generated code.
- All input and output interfaces are controllable. not dependency on environment variables or external files.
- Predictable memory management - Save and restore memory snapshots instead of a garbage collector.
- Runs compiled (byte-code) .pyc files produced by the standard CPython
- Can be optionally linked to CPython for the purpose of compiling python code into bytecode and running an interactive interpreter.
- Python code can easily interface with native C++ using an interface similar to boost::python
- Written in standard C++11.


Building
========

- Open PyVM.sln using Visual Studio 2015 (Community edition can be downloaded for free)

- To build with linking to CPython open src/pythonRoot.props with a text editor and changed 
    <PYTHONROOT>C:\Python27</PYTHONROOT>
  to point to where your installation of Python 2.7 resides.
  From Visual Studio this can be reached from -
    VIEW -> Property Manager -> project libPyVM -> "Debug|Win32" -> double click "pythonRoot" -> User Macros
  (This changes the root for all configs, not just Debug|Win32)

- To build without CPython change in src/pythonRoot.props the variable USE_CPYTHON to 0
  From Visual Studio -
    VIEW -> Property Manager -> project libPyVM -> "Debug|Win32" -> double click "pythonRoot" -> C/C++ -> Preprocessor
    in "Preprocessor Definitions" change USE_CPYTHON to 0

- Build the solution
- Run the tester
  in the tester main.cpp you can choose between different main() functions which demonstrate aspects the the interpreter
  to run them, change main() to _main() and one of the other functions to be main()


To build in earlier versions of Visual studio (up to 2010), open the project properties window, under 'General', change 'Platform Toolset' to the version of Visual Studio that you have.