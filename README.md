# ImGUI version of uCAN Monitor

## Prerequisites to build

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

A server's `src/ucanopen_servers/<client>/<client>_od.cpp` can be generated
from the object dictionary table of its firmware (the `od_row` rows in the
firmware's `od.cpp`, emblib's compile-time dictionary). Regenerate it after
the firmware's table changes instead of editing it:

```bash
scripts/generate_od.sh adptetk ~/dev/adpt-etk-inverter/src/app/inverter/comm/can/canopen/od.cpp
```


## Prerequisites to run

* **Ubuntu 22.04**

```bash
sudo apt install libglfw3
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt install g++-13
```
