# Xenon

Xenon is a native Ubuntu system utility suite built with C++20, GTK4, and Libadwaita. Its desktop-style interface combines monitoring and system management tools in one focused workspace.

<img width="632" alt="xenon" src="https://github.com/user-attachments/assets/f9c0e333-bf0f-4638-84b1-36c13a383387" />


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

- Resource Monitor — live CPU and memory usage from `/proc`, including usage history graphs.
- Process Manager — lists running processes with PID, state, and resident memory; selected processes can receive a termination request.
- Services Manager — lists systemd service enablement and runtime state; selected services can be started, stopped, enabled, or disabled with system authorization.
- System Cleaner — selectively clears APT package archives, crash reports, rotated logs, application caches, and desktop trash.
- Repository Manager — adds, enables, disables, and deletes APT source files with system authorization.

# License
This repository is licensed under the **Unlicense** license.\
For more information, click [here](https://unlicense.org/).
