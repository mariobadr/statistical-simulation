# HRD Model Generator

The executable allows you to serialize an `hrd::profile` based on a trace of memory requests.

## Dependencies

The model-generator uses [https://developers.google.com/protocol-buffers/](Google protocol buffers) (tested with version 3) and [https://github.com/gabime/spdlog](spdlog).
For Google's protobuf, make sure both the `protoc` executable and the library are installed.
The model-generator relies on [https://github.com/mariobadr/ioproto](ioproto), a light wrapper around Google's protobuf library.

## Run

The model-generator produces a `create-hrd-profile` binary.
The binary is responsible for parsing a gem5 packet trace (in a protobuf format) and generating a model (also in a protobuf format).
Run the executable with the help flag (`-h`, `--help`) to see the available options.

....
Create an HRD model from a memory trace.

create-hrd-profile [options] ARG [ARG...]

    -h, --help
        Display help information.
    -i, --input
        gem5 packet trace.
    -o, --output
        Output file.
    -l, --layers
        Layers of the hierarchy (default: 64,4096)
....

