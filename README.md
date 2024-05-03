# Akhenaten [![Github Actions](https://github.com/dalerank/Akhenaten/workflows/Akhenaten%20Build%20Windows/badge.svg)](https://github.com/dalerank/Akhenaten/actions)

[![Discord](https://github-production-user-asset-6210df.s3.amazonaws.com/918081/263684745-bcca8b70-13c4-48d8-8e91-4b0be8a440e6.png)](https://discord.gg/HS4njmBvpb)
[![Download](https://github-production-user-asset-6210df.s3.amazonaws.com/918081/263685010-cd624917-786d-487b-89c0-298bc694f3f2.png)](https://dalerank.itch.io/Akhenaten)
[![Windows](https://github-production-user-asset-6210df.s3.amazonaws.com/918081/263685266-d429392e-a91e-4233-b496-3863e50af5f6.png)](https://nightly.link/dalerank/Akhenaten/workflows/akhenaten_windows/master/windows_build.zip)
[![Linux](https://github-production-user-asset-6210df.s3.amazonaws.com/918081/263685605-8ce46564-04e7-45a7-afa4-0ffe32335dd8.png)](https://nightly.link/dalerank/Akhenaten/workflows/akhenaten_linux/master/linux_build.zip)
[![Mac](https://github-production-user-asset-6210df.s3.amazonaws.com/918081/263685850-a5f39f18-0220-411b-bb70-9bdbc9d48311.png)](https://nightly.link/dalerank/Akhenaten/workflows/akhenaten_mac/master/macos_build.zip)
[![Android](https://github-production-user-asset-6210df.s3.amazonaws.com/918081/268461479-834ae5f0-f57d-4105-b499-869982383a87.png)](https://nightly.link/dalerank/Akhenaten/workflows/akhenaten_android/master/apk.zip)

![2023-08-19 21_17_32-Window](https://github.com/dalerank/Akhenaten/assets/918081/dd616847-9f79-4a01-84fb-800f1cfa2c99)

Akhenaten aims to make the original game Pharaoh compatible with modern systems with redesigned original engine.
Unlike the original game, which was developed by Impressions Games, Akhenaten is a community-driven effort to keep the game alive and accessible.
Allows you to load original save games from Pharaoh and play the initial campaign missions without major issues.

This is a fork of the **Julius/Augustus** project with the aim of making it work with _Pharaoh_ instead of _Caesar 3_.

The work is still in progress, so any help or support is appreciated. You can more or less load
original save games and play the first few missions in the campaign without major issues.

If you're looking for _Caesar 3_'s vanilla experience with modern machine compatibility, check out [Julius](https://github.com/bvschaik/julius).<br>
If you want additional features and visual/UI improvements, and don't care about strict
compatibility with the original save files, check out [Augustus](https://github.com/Keriew/augustus).<br>
For the original game, check out the page on [Steam](https://store.steampowered.com/app/564530/Pharaoh__Cleopatra/)
or [GOG](https://www.gog.com/en/game/pharaoh_cleopatra).<br>
For the official upcoming remaster (unrelated to Akhenaten) called _A New Era_, check out the Steam page [here](https://store.steampowered.com/app/1351080/Pharaoh_A_New_Era/).

## Running the game

| Platform       | Latest release | Unstable build |
| -------------- | -------------- | -------------- |
| Windows        | -              | [![Github Actions](https://github.com/dalerank/akhenaten/workflows/Akhenaten%20Build%20Windows/badge.svg)](https://nightly.link/dalerank/Akhenaten/workflows/akhenaten_windows/master/windows_build.zip)  |
| Linux AppImage | -              | [![Github Actions](https://github.com/dalerank/akhenaten/workflows/Akhenaten%20Build%20Linux/badge.svg)](https://nightly.link/dalerank/Akhenaten/workflows/akhenaten_linux/master/linux_build.zip)        |
| Mac            | -              | [![Github Actions](https://github.com/dalerank/akhenaten/workflows/Akhenaten%20Build%20Mac/badge.svg)](https://nightly.link/dalerank/Akhenaten/workflows/akhenaten_mac/master/macos_build.zip)            |
| Android        | -              | [![Github Actions](https://github.com/dalerank/akhenaten/workflows/Akhenaten%20Build%20Android/badge.svg)](https://nightly.link/dalerank/Akhenaten/workflows/akhenaten_android/master/apk.zip) |
| Flatpak        | -              | [![Github Actions](https://github.com/dalerank/akhenaten/workflows/Akhenaten%20Build%20Linux%20%28Flatpak%29/badge.svg)](https://nightly.link/dalerank/Akhenaten/workflows/akhenaten_flatpak/master/akhenaten.flatpak.zip) |



After downloading the most recent binaries from above or building them from source,
start Akhenaten and it will ask you to point to an original Pharaoh installation folder.

Akhenaten, like Julius and Augustus, requires the original assets (graphics, sounds, etc)
from an unmodified game installation to run, in this case it needs _Pharaoh_ **as well as the _Cleopatra_ expansion.**

Note that you must have permission to write in the game data directory as the saves will be
stored there; also, your game must be patched to last version to run Akhenaten.

## Building Akhenaten from source

### Windows + Visual Studio

- Clone the repository
- Install [CMake](https://cmake.org/download/#latest)
- run update-workspace.bat, which download all SDL2 dependencies and create VS solution for you

### Windows + Yours IDE

To build with your favorite IDE, just import the cmakelists.txt file as a project and build from there. Otherwise, you can use [MinGW-w64](https://www.mingw-w64.org/downloads/) to build via CLI:

- Clone the repository
- Install [CMake](https://cmake.org/download/#latest)
- From the project's root folder execute, in order:
  
  ```
  mkdir build
  cd build
  cmake .. -G "MingGW Makefiles"
  mingw32-make
  ```

To run the engine, you'll also need the necessary dynamic libraries in the same folder as the executable, unless the project is set to build with static linkage.

- Navigate to the SDL2, SDL2_mixer and SDL2_image extracted files, and locate the .dll files inside the `bin` folder of the correct architecture (e.g. `SDL2_image-2.6.2/x86_64-w64-mingw32/bin/`)

- Copy the .dll files from each of the above to the `build` folder

### Linux

Only tested on Manjaro, but under Ubuntu or similar you should install the same packages:

- Clone the repository

- From the root folder execute:
  
  ```
  $ ./update-workspace-linux.sh
  $ cmake --build ./build --target clean
  $ cmake --build ./build
  ```

### MacOS

Only tested on Manjaro, but under Ubuntu or similar you should install the same packages:

- Clone the repository

- From the root folder execute:
  
  ```
  $ ./update-workspace-mac.sh
  $ cmake --build ./build --target clean
  $ cmake --build ./build
  ```

### Android

- Clone the repository

- From the root folder executr:

  ```
  sudo apt install openjdk-17-jdk openjdk-17-jre ninja-build
  sudo wget https://dl.google.com/android/repository/commandlinetools-linux-9123335_latest.zip
  sudo unzip commandlinetools-linux-9123335_latest.zip
  cd cmdline-tools/bin
  sdkmanager --install "platform-tools" "build-tools;30.0.1" "emulator" "platforms;android-33"
  cd ../../build
  cmake -DCMAKE_BUILD_DEPENDENCIES=android ..
  cd ../android
  ./gradlew assembleDebug
  ```

## Existing build options

### Building with logging stack trace on crash

Checkout `cpptrace` submodule:

```shell
git submodule init
git submodule update ext/cpptrace
```

Build in Debug mode.

### Running with different log levels

Use environment variable `SDL_LOG_PRIORITY` for adjustment of logging. For example:

```shell
SDL_LOG_PRIORITY=debug
```

By default `info` level is set.

### Running with tracy

Enable tracy submodule:

```shell
git submodule init
git submodule update ext/tracy
```

Run profiler:

```shell
cd ext/tracy/profiler/build/unix && make && ./Tracy-release
```

Build & run with tracy enabled:

```shell
cmake -DUSE_FUTURE_TRACY=ON .. && make akhenaten && ./akhenaten
```

![Alt](https://repobeats.axiom.co/api/embed/99a27c096522f0ed847ec37c6495d79552aeb13e.svg "Repobeats analytics image")
