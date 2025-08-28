# Module template

This topic provides a module template with a minimal CMakeLists.txt and a source file. You can use it as a template for creating a new module or as a reference. In any case, I recommend you carefully study what is written here.

## Architecture

The module contains a CMakeLists.txt file and source code:

```
.
├── CMakeLists.txt
└── src
    └── main.c
```

## CMakeLists.txt

CMakeLists.txt is next:

```cmake
cmake_minimum_required(VERSION 3.28)

# Set your module name here
set(MODULE_NAME "MyWonderfulModule")

# Do not change this
project(${MODULE_NAME}_module C)

# Create a shared library target
add_library(${MODULE_NAME} SHARED src/main.c)

# Ensure Position Independent Code for shared object
set_target_properties(${MODULE_NAME} PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  PREFIX ""        # Remove the "lib" prefix so output is MyWonderfulModule.so instead of libMyWonderfulModule.so
  OUTPUT_NAME ${MODULE_NAME}
)

# Compiler flags
target_compile_options(${MODULE_NAME} PRIVATE
    -fPIC
    -pedantic
    -Wall -Wextra
    -Wcast-align
    -Wunused
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wlogical-op
    -Wuseless-cast)

# Link with SDK
target_link_libraries(${MODULE_NAME} PRIVATE smu-server-sdk-c)
```

## main.c

This is the main source file of our module. You can give it any name or leave it as is.

**Click on the function name to read its purpose and documentation.**

\include{.c} examples/MyWonderfulModule/src/main.c


## Result 

After compilation and installation on the server, when you start the server you will see:

```
[28.08.25 17:06:28] Registering an external module with name "MyWonderfulModule"...
[28.08.25 17:06:28] [MODULE MyWonderfulModule] Greetings from a module written in C!
[28.08.25 17:06:28] [MODULE MyWonderfulModule ("Example module")] Registered (RUNNING)

```

And when the server is shut down:
```
[28.08.25 17:07:41] [MODULE MyWonderfulModule] My wonderful module is being destroyed. Bye-bye :)
```

## Conclusion

This topic showed a module template. In the [next one](./1_own_module.md) we will look at creating modules in more detail.


