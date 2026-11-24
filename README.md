# AWS Lambda C Runtime

C implementation of the [AWS Lambda Runtime API](https://docs.aws.amazon.com/lambda/latest/dg/runtimes-api.html).

## Frequently Asked Questions

### Why do this?

Why not?

### No really, why do this?

I believe AWS Lambda is one of Amazon's most significant innovations since the company launched as an online bookstore. Since the original website was written in C, this project serves as an homage to Amazon's history.

### Is the code safe?

Absolutely not. But it works.

### How is this tested?

An example Lambda handler is provided in `examples/hello/`. A test file is also provided which runs the example against a mock AWS Lambda runtime API built with Node.js.

#### Process

1. Build the library: `make lib`
2. Build an example: `cc -Iinclude examples/hello/main.c -Lbuild/lib -llambda -o build/hello`
3. Run the tests against it (Node.js 24 or newer): `BOOTSTRAP=build/hello node --test examples/hello/test.js`

With Nix, `nix flake check` builds everything and runs the tests.

### How fast is it?

Very fast.

![screenshot of successful invocation of the Lambda function showing a duration of 1.13ms and an init duration of 7.05ms](media/screenshot.png)

## Usage

Write a handler and pass it to `lambda_run()`. See `include/lambda.h` for the API and `examples/hello/main.c` for a complete function.

### With pkg-config

Install the library, header, and pkg-config file (to `/usr/local` by default, or set `PREFIX`):

```sh
make install PREFIX=$HOME/.local
```

Then build your function against it:

```sh
cc main.c $(pkg-config --cflags --libs lambda) -o bootstrap
```

Any build system that understands pkg-config works the same way. See `examples/hello/CMakeLists.txt` for CMake usage.

### With Nix

Add this repository as a flake input and apply `overlays.default`, which provides `aws-lambda-c`. Use `pkgsStatic.aws-lambda-c` to build a static binary that runs on any OS-only Lambda runtime. `examples/hello/package.nix` shows a complete function.

To build the example as a zip ready to upload (requires a Linux builder):

```sh
nix build .#packages.x86_64-linux.example-hello
```

The zip is at `result/lambda.zip`.

### Deploying

Lambda runs the executable named `bootstrap` at the root of the zip, using an OS-only runtime such as `provided.al2023`.

A binary dynamically linked against glibc must be built against a glibc no newer than the runtime's, for example by building on Amazon Linux 2023 for `provided.al2023`. A statically linked binary, such as one built with `pkgsStatic`, has no such requirement.

## License

Licensed under the [MIT License](LICENSE).
