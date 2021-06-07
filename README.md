# Simple Marvis ITS Library

This library provides ITS functions on the basis of the MobilityProvider in Marvis [https://github.com/Paul-Geppert/marvis](https://github.com/Paul-Geppert/marvis).

Following functions are implemented:

* a simple Cooperative Awareness Service, which requests the vehicle status and sends CAM messages periodically
* a function to create and send SPATEM messages filled with the provided input

The ITS messages are already compiled from their definition in `./asn_files` by using `asn1c`.

## Requirements

To compile the library, you need following requirements:

* C compiler, e.g. `gcc`
* `make`
* a XML-RPC-C library, for example `libxmlrpc-core-c3-dev`

## Build

### Build the library

To compile simply call `make -j$(nproc)`.

### Compile the asn files

The asn files are already compiled, but if you want to do it yourself, find more information here.

To compile the asn files, you need to install `asn1c` [https://github.com/mouse07410/asn1c](https://github.com/mouse07410/asn1c).
**Please notice, that this is not the original repo, but a forked version!** This version include several approvements and fixed of the original project. It periodically merges the `main` branch of the original repository into this project.

You can find additional requirements and instructions for install in their README.

Simply run `./build.sh`. Using parameter `-c` (or `--clean-start`) will clean the environment first (removing temporary and generated files). When usig parameter `-d` (or `--debug`) will cause compilation with debug flags, the application will later produce low level verbose debug output.

## Questions, feedback

Feel free to give feedback or send questions anytime: [paul.geppert@student.hpi.de](mailto:paul.geppert@student.hpi.de).
