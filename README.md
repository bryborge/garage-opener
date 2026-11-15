# Garage Door Opener

This is the final project for ECE361 - Computer System Organization at Portland State University. It is an embedded system designed to behave like a garage door opener. It uses the `iom361` library, version 4.0 (r4), built specifically for this class to imitate an I/O device attached to a microcontroller.

## Specification

The project assignment details are captured in the specification document found in [docs/SPEC.md](docs/SPEC.md).

## Getting Started

This project is built with [Meson](https://mesonbuild.com/). It builds the `garage_opener` program and test suite.

### Requirements

- A C11 compiler
- Meson 1.1 or later
- Ninja

### Installing

```sh
meson setup build
meson compile -C build
```

The program executable is in `build/garage_opener`.

### Testing

The [Unity](https://github.com/ThrowTheSwitch/Unity) test framework is used by Meson under the hood.

```sh
meson test -C build
```
