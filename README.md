# <center>AzureFlare</center>
AzureFlare is a drop in DLL mod for the official Japanese PSOBB client to allow opening the game without the need of modifying the PsoBB.exe executable.

## What it can do?
At the time of writing this README:
* Server redirection for US, JP, Ep4 and CN modes respectively
* GameGuard bypass (Both packed and unpacked EXE)
* Allow changing the language of the game so it can load the US localization
* Allow writing in game chat without the need of installing a Japanese IME
* Enabling or disabling Episode 4 mode

## What it won't do?
Anything that isn't making the game work on systems and features to make it work on non Japanese systems.

## To do
* You tell me! Please [open an issue](https://github.com/Repflez/AzureFlare/issues/new) for your requests.

## Why AzureFlare?
I looked for synonyms of Blue and Burst repectively and took 2 at random.

## Download
Head on to the [Downloads page](https://github.com/Repflez/AzureFlare/releases/latest).

## Installation
Drop `wsock32.dll` and `psobb.cfg` in your game folder, where `PsoBB.exe` is in.

## Building
1. Install [MSYS2](https://www.msys2.org/) or install the MinGW packages for building x86 apps from your distro's package manager.
1. Install [Premake](https://premake.github.io/) anywhere accessible by your PATH.
3. Inside the AzureFlare folder, generate the Makefiles using the `premake5 gmake` command.
4. Inside the newly generated `build` folder, you can just `make` to get a working build. To build without debug features, you can add `config=release_x86` to the `make` command.