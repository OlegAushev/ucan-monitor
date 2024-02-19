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

## Prerequisites to run

* **Ubuntu 22.04**

```bash
sudo apt install libglfw3
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt install g++-13
```
