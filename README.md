
![GitHub repo size](https://img.shields.io/github/repo-size/cm8/d2tm?style=plastic)
![GitHub language count](https://img.shields.io/github/languages/count/cm8/d2tm?style=plastic)
![GitHub top language](https://img.shields.io/github/languages/top/cm8/d2tm?style=plastic)
![GitHub last commit](https://img.shields.io/github/last-commit/cm8/d2tm?color=red&style=plastic)

Dune II - The Maker
===================
A.k.a. D2TM. Is a [Dune 2](http://en.wikipedia.org/wiki/Dune_II) remake.

See [Dune-II---The-Maker](https://github.com/stefanhendriks/Dune-II---The-Maker) for the origin of this fork.

Read [the wiki](https://github.com/stefanhendriks/Dune-II---The-Maker/wiki) for more information about startup params, controls, and more.

## Compiling

### Prerequisites
- `CMake` version 3.10 or higher
- `GCC` version 7.5 or higher

Make sure you can run `make` from command line.

### Ubuntu / Unix
- git clone this project
- run `./install_dependencies_ubuntu.sh` (only required once)
- create a `build` dir
```
    mkdir build
    cd build
    cmake ..
    make -j4
    cd ..
    ./create_release.sh
```

### Windows (MinGW)
It currently depends on MinGW32. Make sure you also have GCC 11 installed,
which you can do via MSYS2 (using `pacman`). GCC 11 is not installed by
default in MinGW32!

- git clone this project
- create a `build` dir
```
    mkdir build
    cd build
    cmake .. -G "MinGW Makefiles"
    cmake --build . --target all -- -j 6
```

If you use `ninja` you can use `cmake .. -G Ninja` instead.

Once compilation is done you'll end up with a `d2tm.exe` file and several DLL's.

## Running

### Ubuntu / Unix
Run `./d2tm` from the release folder (created by `./create_release.sh`, see Compiling).

### Windows
Easiest is to run the executable from the root.
- Shared libraries required are: libalfont.dll, alleg42.dll, libwinpthread-1.dll and mscvr70.dll.
- Required pre-built library (Allegro 4) is in the [dll folder](https://github.com/cm8/d2tm/tree/master/dll/mingw32).
- The library that is built with the project (AlFont) is copied to the build directory.
- libwinpthread-1.dll may be on your MinGW path. If it is not on the path, copy it from your mingw32 installation.

## Binaries
This fork does not offer releases, so the only option at the moment is
- Compile this project yourself

## Topics
In the code there are several known concepts; see below for extra information
about them:

- [Angles: how to calculate and use them to draw things](doc/convertAngleToDrawIndex.md)

## Troubleshoot
In case of non-working MIDI sound, you can choose from three alternative driver options configurable in `d2tm.cfg`. The more detailed this configuration, the lesser autodetection routines are run internally to find a working setup.

This also means if `d2tm.cfg` is misconfigured or inappropriate for the interpreting machine, in spite of autodetection possibly finding a working setup, sound will not work. To revert to autodetection empty the `[sound]` section in `d2tm.cfg` (place for this project's allegro config options) or comment all entries in that section by prepending `;` like shown below.

Only one `midi_card` entry, if at all, should be active. If you decomment `midi_card = ASEQ` entry, allegro will skip detecting other midi drivers. If `midi_aseq_*` are unset / stay commented, client and port will be autodetected. Check `log.txt` for the values. `VirMidi` and `Midi Through` ports are skipped by autodetect, if you need them configure `midi_aseq_*` options manually.

A prerequisite to using `midi_card = DIGI`, a wavetable synth built into allegro, is a file `patches.dat`. Tool `pat2dat` of allegro is capable of generating such from sf2 soundfonts in case it is missing.  For further help on `midi_card = AMID` consult the allegro docs.

```ini
[mouse]
mouse_accel_factor=0

[sound]
;midi_card = DIGI
;patches = /path/to/patches.dat

;midi_card = ASEQ
;midi_aseq_client = 128
;midi_aseq_port = 0

;midi_card = AMID
;alsa_rawmidi_device = hw:2,0
```