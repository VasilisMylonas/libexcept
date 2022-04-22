# libexcept

Exception handling mechanism for C.

This library provides a way of signaling and handling errors similar to other languages in the form of the try/catch mechanism.

## Features

- try/catch constructs for handling exceptions.
- finally clause for ensuring resource cleanup.
- throw/rethrow statements for throwing exceptions.
- various function hooks for customizable behavior.
- optional handling of signals as exceptions.
- compatible with the traditional C strategy of using integer values as error codes.
- small size.
- optional thread awareness.
- inline documentation with examples.

## Requirements

For building the project CMake version 3 or newer and a compiler supporting C11 features is required.

## Building

Run the following commands, replacing [source directory] with the cloned repo and [build directory] with the directory to output the build artifacts.

To generate the build system:

`cmake -S [source directory] -B [build directory]`

To build the project:

`cmake --build [build directory]`

Additionally, the following options may be added to the generation command to alter specific behavior:

- Enable for multi-threaded use (default is ON): `-DLIBEXCEPT_THREAD_AWARE=ON/OFF`

- Enable for signal catching (default is ON): `-DLIBEXCEPT_SIGNAL_AWARE=ON/OFF` NOTE: on Windows this option is disabled due to the unavailability of POSIX signal APIs

## License

[MIT](./LICENSE.txt)
