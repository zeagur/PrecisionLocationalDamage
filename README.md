# Precision Locational Damage NG

You can build a project with CMake by using the following steps.

First, make sure you have installed CMake on your system. If not,
you can download it from the official website and follow the instructions provided 
there for your specific operating system.

To build a CMake project, you need a `CMakeLists.txt` file which directs the build process. 
This should already be present if you have a `CMake-based` project.

Here's the general process:
1. install required packages and libs using `vcpkg` json file
2. Open a command prompt/terminal and navigate to your project directory.

3. Create a new directory (usually build) and move into this directory (Because it is a good practice to do an "out-of-source" build rather than build in your source directory):

    ```shell
    mkdir build
    ```

4. Run cmake `{options} .` from this directory

    ```shell
    cmake . -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg
    ```

After running cmake, run `cmake --build .` to actually build your project. 
You can use `--config Release` to specify a build type.

5. build with cmake profile `DLL` file for SKSE

    ```shell
    cmake --build . --config Release/Debug
    ```