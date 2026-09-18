# Xenon

Xenon is a native GNOME system monitor for Ubuntu, built with C++20, GTK4, and Libadwaita.

## Building

Xenon targets Ubuntu 24.04 LTS. Install the build dependencies:

```bash
sudo apt install build-essential meson ninja-build pkg-config libgtk-4-dev libadwaita-1-dev
```

Configure and build the project:

```bash
meson setup build
meson compile -C build
```

Run Xenon from the build directory:

```bash
./build/src/xenon
```

Run the automated tests:

```bash
meson test -C build --print-errorlogs
```

## Features

- Resource Monitor: live CPU and memory usage from `/proc`.
- Process Manager: lists running processes with PID, state, and resident memory; selected processes can receive a termination request.

## TODO

- Services Manager
- System Cleaner
- Repository Manager

# License
This repository is licensed under the **Unlicense** license.
For more information, click [here](https://unlicense.org/).