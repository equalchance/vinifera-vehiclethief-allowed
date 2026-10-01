# Build it

Use the x86 Native Tools prompt from Visual Studio 2022 Build Tools, with CMake,
Ninja, and Git installed. The shared build used MSVC 14.44.35207,
`RelWithDebInfo`, and `PREVIEW=ON`.

The public binary uses a neutral build folder. If `V:` is free, this example
sets one up. Keep the mapping active until the build finishes:

```bat
mkdir C:\ViniferaPublic
subst V: C:\ViniferaPublic
git clone https://github.com/equalchance/vinifera-vehiclethief-allowed.git V:\ViniferaSrc\_build\vinifera-vehiclethief-allowed
cd /d V:\ViniferaSrc\_build\vinifera-vehiclethief-allowed
```

From that repository's root:

```bat
git clone --no-checkout https://github.com/Vinifera-Developers/Vinifera.git source
git -C source checkout --detach a02a90f816328ac59f94b455982d22979fed1de1
git -C source submodule update --init --recursive
git -C source apply --check ..\VehicleThief.Allowed.patch
git -C source apply ..\VehicleThief.Allowed.patch
mkdir symbols
cmake -G Ninja -S source -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DPREVIEW=ON -DMINIAUDIO_FORCE_CXX=ON -DMINIAUDIO_NO_ENCODING=ON -DMINIAUDIO_NO_GENERATION=ON -DMINIAUDIO_NO_EXTRA_NODES=ON -DCMAKE_SHARED_LINKER_FLAGS=/PDBSTRIPPED:../symbols/feature.pdb
cmake --build build --config RelWithDebInfo --parallel 6
```

Keep the submodule commits listed in `pins.json`. The resulting
`build/Vinifera.dll` and `symbols/feature.pdb` must come from the same link.
Install the stripped `feature.pdb` beside the DLL as `Vinifera.pdb`, matching
the filename in the DLL's debug record. After building, leave
the mapped folder and run `subst V: /d` to remove the mapping.

The public DLL was checked against the tested build. Executable instructions,
initialized data, hooks, and resources match after accounting for neutral
source paths, debug metadata, and references to reordered audio globals.
`checks/public-binary-equivalence.json` records the comparison. Builds on another
system can have different hashes because of timestamps, generated Git/SDL
version strings, compiler ordering, and debug paths. A hash difference alone
doesn't establish equivalent behavior.

The source download also includes the patched Vinifera sources and the pinned
TSpp, SDL, imgui, and miniaudio sources. The clone-and-patch commands above retain
upstream Git metadata. The tested build's generated TSpp Git fields were empty;
a fresh build may report those fields in crash information.

Use the matched no-spawner executable and launcher for this Vinifera branch.
Vinifera supplies the spawner. Their SHA256 values are in `pins.json`; the
download does not include a complete game installation. Apply the source patch
and rebuild for a mod using a different branch or executable.

The save format changes. Same-build saves work; older saves are rejected, with
no migration. Keep existing installs and saves separate from this test build.
