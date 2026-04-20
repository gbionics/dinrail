# Examples

This directory contains standalone example projects that consume `dinrail` as an external package.

## Available examples

- `implement-your-first-device/`: implement a minimal device plugin and load it at runtime with `dinrail::Device`.

## Run examples with Pixi

Examples are managed by their own Pixi workspace manifest: `pixi.toml`.

From the project root, move to the examples folder first:

~~~
cd examples
~~~

Then you can use:

~~~
pixi run build
~~~

to configure and build the examples.

Then:

~~~
pixi run install
~~~

to install them into the examples Pixi environment.

To see how the installed examples devices are found by dinrail, run:

~~~
pixi run dinrail dev --list
~~~

To build, install, and run the `implement-your-first-device` executable in one command:

~~~
pixi run run-implement-your-first-device
~~~

## Run examples against an external dinrail

If `dinrail` is already installed on your system, you can build and run these examples as a regular CMake project, using the package management tool that you prefer.

From the repository root:

~~~
cd examples
~~~

Then configure and build:

~~~
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
~~~


Install and run:

~~~
cmake --install build --prefix .install-external
.install-external/bin/implement-your-first-device
~~~

If the examples are installed in a different prefix than `dinrail`, set `DINRAIL_PLUGIN_PATH` so the runtime can find the example device plugin:

~~~
DINRAIL_PLUGIN_PATH=.install-external/lib \
	.install-external/bin/implement-your-first-device
~~~


