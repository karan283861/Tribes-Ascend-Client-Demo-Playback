# Tribes-Ascend-Client-Demo-Playback

## Cloning

> git clone git@github.com:karan283861/Tribes-Ascend-Client-Demo-Playback.git --recurse-submodules

## CMake generation

> cmake -S . -B x86 -A Win32 -T v142

## Building

> cmake --build x86 --config=RelWithDebInfo

## Requirements
1. Microsoft Visual C++ Redistributable 2019 (x86)

## Usage
Place demo files under C:\Users\User\Documents\My Games\Tribes Ascend\TribesGame\Demos

Description of demo console commands
| Command | Description| Example|
|--|--|--|
|demoplay| Open a demo file for playback  | demoplay test.demo  |
|pause| Pause or resume playback  | pause  |
|slomo| Slow down or speed up playback by a factor  | slomo 0.1, slomo 1, slomo 10  |
|demorewind| Rewind demo playback  | demorewind 10  |
