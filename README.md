# Open Remote Sensing AI Library Markup Tools (OpenRSAI-Markup)

## Description

OpenRSAI-Markup complies the [OpenRSAI Core](https://github.com/miron77s/open_rsai) with dataset preparing toolchains capable to create Yolo and MS COCO annotations.

## Tutorial

The OpenRSAI-Markup is the [Open Remote Sensing AI Library Core](https://github.com/miron77s/open_rsai) library extension that can be installed and tested in complex according the [tutorial](https://github.com/miron77s/open_rsai/blob/main/tutorial/TUTORIAL.md). 


## Requirements

All instructions below are tailored for Ubuntu 22.04 users. If you are using a different operating system, please adjust the commands accordingly.

The requirements are the following:

1. Dataset preparing toolchain uses basic utilities of [Open Remote Sensing AI Library Core](https://github.com/miron77s/open_rsai), so follow it's [requirements](https://github.com/miron77s/open_rsai#requirements).

2. Install Qt5 packages:

```
sudo apt-get install -y qtbase5-dev qtdeclarative5-dev
```

3. Pull OpenRSAI-Markup submodules:

```
git clone https://github.com/miron77s/open_rsai_markup
cd open_rsai_markup
git submodule update --init --recursive
```

### Hardware

OpenRSAI-Markup hardware requirements are dictated by [OpenRSAI-Core](https://github.com/miron77s/open_rsai#hardware). The dataset generation tools themselves require 8Gb RAM.

## Compile and Installation (using CMake)

Follow the OpenRSAI Core [build guide](https://github.com/miron77s/open_rsai/blob/main/README.md#compile-and-installation-using-cmake) to build and install a toolchain.

## Special Thanks

We wish to thank Innovations Assistance Fund (Фонд содействия инновациям, https://fasie.ru/)
for their support in our project within Code-AI program (https://fasie.ru/press/fund/kod-ai/).
