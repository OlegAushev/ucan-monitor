ImGUI version of uCAN Monitor

### Dependencies

* **Fedora:**

```bash
sudo dnf install cmake gcc-c++ glfw-devel boost-devel clang-tools-extra
```

* **Ubuntu**

```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt install g++-13 cmake libglfw3-dev libboost-all-dev
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 60 --slave /usr/bin/g++ g++ /usr/bin/g++-13
```
