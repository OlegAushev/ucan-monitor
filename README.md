# ImGUI version of uCAN Monitor

On the branch develop the program serves the adpt-etk-inverter alone, on
its firmware after the CANopen rework (`docs/canopen-rework-plan.ru.md`);
the other devices stay on master.

## Prerequisites to build

The description of a dictionary is read with Boost.JSON, header-only, so
Boost must be 1.75 or newer: the 1.74 of Ubuntu 22.04 is older.

* **Fedora:**

```bash
sudo dnf install cmake gcc-c++ glfw-devel boost-devel clang-tools-extra
```

* **Ubuntu 22.04**

```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt install g++-13 cmake libglfw3-dev libboost-all-dev
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 60 --slave /usr/bin/g++ g++ /usr/bin/g++-13
```

## Build flatpak

```bash
sudo dnf install flatpak-builder
flatpak install flathub org.freedesktop.Platform//23.08 org.freedesktop.Sdk//23.08
# flatpak install flathub org.gnome.Sdk
flatpak-builder build-dir org.flatpak.ucan-monitor.yml
```


## Object dictionaries

A server reads the description of its device's object dictionary at
startup, `assets/od/<device>.od.jsonl`: the file that the firmware's build
extracts from its ELF (emblib, `docs/canopen-od-algorithm.ru.md`, section
3.9). The objects, the PDOs and the statuses all come from it. Take a new
one in when the firmware's dictionary changes:

```bash
scripts/update_od.sh ~/dev/adpt-etk-inverter rev-a-release
```

The script checks the description with `od_decode.py --check` of the
firmware's own emblib first, and `ctest --test-dir build` then checks that
the program reads it and decodes the firmware's example of frames as
`od_decode.py` does. At each connection the program compares the device's
fingerprint, 2000h:00, with the description's and warns when they differ.


## Prerequisites to run

* **Ubuntu 22.04**

```bash
sudo apt install libglfw3
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt install g++-13
```
