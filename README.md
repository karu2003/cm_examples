# Example OOT project for Dev Board Micro

This serves as a starting point for your own
Dev Board Micro projects when you want your project to live outside the 
[coralmicro](https://github.com/google-coral/coralmicro) tree, rather than inside it.
For more information about creating a project, either in-tree or out-of-tree, see the guide
to [Build apps with FreeRTOS for the Dev Board Micro](https://coral.ai/docs/dev-board-micro/freertos/).

**Note:** This project depends on [coralmicro](https://github.com/google-coral/coralmicro),
which requires about 2.5 GB.


## 1. Clone this project and submodules

```bash
git clone --recurse-submodules -j8 https://github.com/karu2003/cm_examples
```

## 2. Build the project

```bash
cd cm_examples

cmake -B out -S .

make -C out -j8
```

To maximize your CPU usage, replace `-j8` with either `-j$(nproc)` on Linux or
`-j$(sysctl -n hw.ncpu)` on Mac.

### 2.1 Build in VS code
use only release configuration


## 3. Flash it to your board

```bash
python3 coralmicro/scripts/flashtool.py --build_dir out --elf_path out/xxx/xxx
```

Anytime you make changes to the code, rebuild it with the `make` command and flash it again.

**Note:** In addition to specifying the path to your project's ELF file with `elf_path`, it's
necessary to specify the build output directory with `build_dir` because flashtool needs to get
the elf_loader (bootloader) program from there.

If you followed the guide to [get started with the Dev Board
Micro](https://coral.ai/docs/dev-board-micro/get-started/), then both the `build_dir` and `elf_path`
arguments are probably new to you. That's because when flashing in-tree examples and apps (as we do
in that guide), the `build_dir` is ommitted because the flashtool uses the local `build` directory
by default. Similarly, in-tree examples and apps don't need to specify the ELF file with `elf_path`
because those files are built in the same build directory—we can instead specify just the project
name with `--example` (or `-e`) and `--app` (or `-a`) when flashing these in-tree projects.

### how to use matplotlibcpp
sudo apt-get install python3-dev

Find the location of the numpy headers. You can do this with a Python one-liner:
python3 -c "import numpy; print(numpy.get_include())"
This will print the path to the numpy headers.
include_directories(/path/to/numpy/headers)

git submodule add -b 3.4.0 https://gitlab.com/libeigen/eigen.git lib/eigen-3.4.0
cd eigen-3.4
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
sudo make install
