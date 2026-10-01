# Apply and build

Use an x86 Native Tools prompt with Visual Studio 2022 Build Tools, CMake,
Ninja, and Git. The supplied build uses MSVC 14.44.35207,
`RelWithDebInfo`, and `PREVIEW=ON`.

From this repository's root, build the pinned upstream version:

```bat
git clone --no-checkout https://github.com/Vinifera-Developers/Vinifera.git source
git -C source checkout --detach a02a90f816328ac59f94b455982d22979fed1de1
git -C source submodule update --init --recursive
git -C source apply --check ..\VehicleThief.Allowed.patch
git -C source apply ..\VehicleThief.Allowed.patch
mkdir symbols
cmake -G Ninja -S source -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DPREVIEW=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DMINIAUDIO_FORCE_CXX=ON -DMINIAUDIO_NO_ENCODING=ON -DMINIAUDIO_NO_GENERATION=ON -DMINIAUDIO_NO_EXTRA_NODES=ON -DCMAKE_SHARED_LINKER_FLAGS=/PDBSTRIPPED:../symbols/feature.pdb
cmake --build build --config RelWithDebInfo --parallel 6
```

Keep the submodule commits in `pins.json`. The source ZIP already contains the
patched source and those submodules; skip the Git commands when using it.

For an existing Vinifera branch, apply `VehicleThief.Allowed.patch` to its
checkout and build with that branch's dependencies and matched runtime.
Resolve any patch conflicts against its code before building.

Install `build/Vinifera.dll` with `symbols/feature.pdb` renamed to
`Vinifera.pdb`. Keep the DLL and PDB from the same link.
[Installation notes](INSTALLING.md) cover runtime compatibility, saves, and rollback.
