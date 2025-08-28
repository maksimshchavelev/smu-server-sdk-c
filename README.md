# What is smu-server-sdk-c?

This is an SDK for developing external dynamically loadable modules in C language as part of the server part of the [smu (server monitoring utility) project](https://github.com/maksimshchavelev/server-monitoring-utility)


## Documentation

[See documentation here](https://maksimshchavelev.github.io/smu-server-sdk-c/html/)


## Installation & Usage (for users)

If you are a regular user, download the **non `dev`** package and install it

> You can download the **non `dev`** package from the `smu` releases - it will contain SDKs for all languages ​​integrated into the project


## Installation & Usage (for developers)
To use, install the `dev` package from releases (the `dev` package only contains runtime libraries). It also contains header files.

> You can download the `dev` package from the `smu` releases - it will contain SDKs for all languages ​​integrated into the project

Next, check out these articles in the guide:

- [Rules for creating your own modules](guides/0_module_template.md)
- [Creating the first module](guides/1_own_module.md)



## Building from source

First, you will need `cmake` and `gcc`. Install them with:
```
sudo apt install cmake gcc
```

Then download the archive with the source code of the latest release and unzip it somewhere. Go to the directory with the source code. **Further, all commands will be executed from this directory**:

```
mkdir build && cd build
```

```
cmake .. -DCMAKE_BUILD_TYPE=Release -DSDK_C_BUILD_TESTS=OFF
```

```
cmake --build . --parallel
```

Then create `deb` packages:

```
cmake --build . --target package
```

You will see a development package and a package with runtime libraries.


### Building flags

The following cmake build flags are available:

Flag | Effect | Default value
--- | --- | --- |
`SDK_C_BUILD_TESTS` | Builds tests if `ON`. Tests can be run via `ctest` from the build directory | `ON`
`SDK_C_ENABLE_SANITIZERS` | Enables sanitizers (address, undefined, leak) if `ON` and build type is `Debug` | `OFF`

Use flags during configuration by passing them to cmake via `-D<flag>`, for example:

```
cmake .. -DSDK_C_ENABLE_SANITIZERS=ON
```


## Contributing
Please follow these guidelines:
- Use `clang-format.txt`, which is located in the root of the repository for code formatting
- Write clear, structured code.
- Variables should clearly indicate their purpose. For global variables in `.c` files, use `static`
- Functions must start with the prefix `sdk`. Then comes the name of the component. Then the name of the function, clearly and unambiguously reflecting its purpose. For example, `sdk_utils_log` refers to the SDK utility functions and is needed for logging.
- Cover your code with tests. Place tests in the `tests` directory. The main test file is `main.c`. In it, write the test as a normal program, using the `Unity` testing library.
- Write all the names of everything in `snake_case`
- Commits should be clear and reflect the essence of the changes.
- Describe all your functions/structures etc. in `Doxygen` style
- Run tests only in **Debug** configuration **with sanitizers enabled**. This will catch more errors


## License
This project is licensed under the [GPLv3 License](./LICENSE)

It uses third-party components:
- [Unity](https://github.com/ThrowTheSwitch/Unity?tab=readme-ov-file) - Library for unit tests in C

See [third party](3dparty/) for details
