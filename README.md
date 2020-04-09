# Statistical Simulation

This repository contains implementations for multiple statistical simulation methodologies.
Each implementation can produce a model file based on a trace of memory requests.
In addition, each implementation can produce a new and synthetic trace of memory requests based on the model file.

This repository also contains tools, utilities and libraries.
You may find these useful for generating traces, simulating traces, and other things related to statistical simulation in computer architecture.

## Dependencies

This repository uses `CMake` to configure how each binary is built (we recommend at least version 3.6).
In addition, this repositry depends on Google's protobuf library and compiler.
For example, if you are running an instance of Ubuntu you will need to install the packages `libprotobuf-dev` and `protobuf-compiler`.

## Compiling Everything

CMake can configure the project for different build systems and IDEs (type `cmake --help` for a list of generators available for your platform).
We recommend you create a build directory before invoking CMake to configure the project (`cmake -B`).
For example, we can perform the configuration step from the project root directory:

	cmake -H. -Bcmake-build-release -DCMAKE_BUILD_TYPE=Release
	cmake -H. -Bcmake-build-debug -DCMAKE_BUILD_TYPE=Debug

After the configuration step, you can ask CMake to build the project.

	cmake --build cmake-build-release/ --target all
	cmake --build cmake-build-debug/ --target all

Once compiled, you will find many executables in the `bin` directory (e.g., `cmake-build-release/bin`).
Use the `--help` argument for more information on the command line interface of each of these executables.

## More Information

Most subdirectories in this repository contain their own README with more information.
