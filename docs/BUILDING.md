# Build it

Use the x86 Native Tools prompt from Visual Studio 2022 Build Tools, with CMake,
Ninja, and Git installed. The shared build used MSVC 14.44.35207,
`RelWithDebInfo`, and `PREVIEW=ON`.

From this repository's root:

```bat
git clone --no-checkout https://github.com/Vinifera-Developers/Vinifera.git source
git -C source checkout --detach a02a90f816328ac59f94b455982d22979fed1de1
git -C source submodule update --init --recursive
git -C source apply --check ..\VehicleThief.Allowed.patch
git -C source apply ..\VehicleThief.Allowed.patch
cmake -G Ninja -S source -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DPREVIEW=ON
cmake --build build --config RelWithDebInfo --parallel 6
```

Keep the submodule commits listed in `pins.json`. To make the public-symbol PDB
used in the download, create a `symbols` folder and add
`-DCMAKE_SHARED_LINKER_FLAGS=/PDBSTRIPPED:../symbols/feature.pdb` when configuring.
The resulting DLL and PDB must come from the same link. Builds on another system
can have different hashes because of timestamps and debug paths.

The source download also includes the patched Vinifera sources and the pinned
TSpp, SDL, imgui, and miniaudio sources. The clone-and-patch commands above retain
the upstream Git metadata used during the tested build.

Use the matched no-spawner executable and launcher for this Vinifera branch.
Vinifera supplies the spawner. Their SHA256 values are in `pins.json`; the
download does not include a complete game installation. Apply the source patch
and rebuild for a mod using a different branch or executable.

The save format changes. Same-build saves work; older saves are rejected, with
no migration. Keep existing installs and saves separate from this test build.
