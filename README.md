# What is smu-server-sdk-c?

This is an SDK for developing external dynamically loadable modules in C language as part of the server part of the [smu (server monitoring utility) project](https://github.com/maksimshchavelev/server-monitoring-utility)


## Installation & Usage (for users)

If you are a regular user, download the **non `dev`** package and install it

> You can download the **non `dev`** package from the `smu` releases - it will contain SDKs for all languages ​​integrated into the project


## Installation & Usage (for developers)
To use, install the `dev` package from releases (the `dev` package only contains runtime libraries). It also contains header files.

> You can download the `dev` package from the `smu` releases - it will contain SDKs for all languages ​​integrated into the project

Next, check out these articles in the guide:

- [Development of the first module](guides/0_first_module.md)



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


## License
This project is licensed under the [GPLv3 License](./LICENSE)

It uses third-party components:
- [Unity](https://github.com/ThrowTheSwitch/Unity?tab=readme-ov-file) - Library for unit tests in C

See [third party](3dparty/) for details
