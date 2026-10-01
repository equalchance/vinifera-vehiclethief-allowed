# Repeat the native checks

These probes run the real engine functions on its main thread, including
forbidden orders assigned directly, proximity theft, targeting, navigation
queues, capture triggers, and save/load. Regression mode covers the additional
aircraft, classification, truce, deployment, and infiltration cases.

Use an isolated copy of your own matched runtime. Keep the published
`Vinifera.dll` and `Vinifera.pdb` unchanged. The probe changes game objects and
exits the process when its checks finish. Never load it in multiplayer or a
normal installation. It skips the briefing dialog and observes entry triggers
and deployment orders; its hook contracts are in
`checks/native-probe-hook-contract.json`.

## Build the probe and launcher

Build the pinned Vinifera source with compile commands enabled, following
[BUILDING.md](BUILDING.md). In an x86 Native Tools prompt, run these commands
from this repository's root:

```bat
python tools\build_native_probe.py --build-dir build --output-dir probe-build
git clone https://github.com/Vinifera-Developers/SyringeEx.git syringe-source
git -C syringe-source checkout --detach d9f4870cc7dea6bb79cb963c2339a3fff7a39071
msbuild syringe-source\Debugger.vcxproj /p:Configuration=Release /p:Platform=Win32 /m
```

The builder takes the include paths and compiler settings from the
`technotypeext.cpp` compile-command entry. It requires the same x86 compiler
environment used for Vinifera. Copy `probe-build/NativeProbe.dll` and
`syringe-source/Release/Syringe.exe` to the isolated runtime beside `game.exe`.
The normal game launcher injects one DLL; this standalone launcher accepts
both explicitly. The diagnostic DLL and its symbols are local build outputs
and are not included in the downloads.

## Execution and cold save/load

Copy `diagnostics/core.map` to the runtime as `spawnmap.ini`, and
`test-kit/spawn-offline.ini` as `spawn.ini`. The core map tags VTAIRYES with
VTALLOWTAG and VTAIRNO with VTDENYTAG so capture-trigger checks have controls.
Copy the binary download's `pins.json` there too. Before launching, check the
executable hash in PowerShell from that runtime folder:

```powershell
$pins = Get-Content .\pins.json -Raw | ConvertFrom-Json
$actual = (Get-FileHash .\game.exe -Algorithm SHA256).Hash.ToLowerInvariant()
if ($actual -ne $pins.matched_runtime.game_exe_sha256) { throw 'The executable does not match the pinned runtime.' }
```

From that runtime folder:

```bat
echo execution>native-probe-mode.txt
Syringe.exe game.exe -i=Vinifera.dll -i=NativeProbe.dll --args="-SPAWN -CD."
```

Wait for the process to exit. Read `native-execution.log`; each check should
report PASS and its final RESULT should report zero failures. Keep the log
before the next run because each launch overwrites it. A missing RESULT, a
FAIL, or a crash is incomplete acceptance.

For the saved forbidden destination, copy `diagnostics/saved-regressions.map`
as `spawnmap.ini`, set `native-probe-mode.txt` to `save`, and run the same
command. This map retains the core objects and adds the regression objects
needed after loading. It writes `NativeForbidden.sav` and exits.
After that process has closed, append these settings to `spawn.ini`:

```ini
LoadSaveGame=yes
SaveGameName=NativeForbidden.sav
```

Set `native-probe-mode.txt` to `load` and launch again. This is a new process
loading the actual save, rather than reusing objects from the save run.
After it exits, repeat with mode `execution` and the load settings retained
to exercise the core execution checks after loading.

## Regression checks

Replace `spawn.ini` with the clean offline settings so it has no load flags.
Copy `diagnostics/regressions.map` as `spawnmap.ini`, set the mode to
`regressions`, and launch with the same command. Check the final RESULT and
every named case.

For regression checks after loading, start with `saved-regressions.map` and
clean offline settings, run mode `save`, then add the load settings above and run
mode `regressions` in a new process. Keep both logs to show that the save and
the loaded regression run completed.

The probes cover native execution with controlled object state. Positive
capture controls use artificial coordinates; they don't prove ordinary
pathfinding or visual behavior. Normal play and multiplayer results are
recorded separately in [TESTING.md](TESTING.md). No private TI build has been
validated by these checks.

## Compare the clean baseline

Build a second checkout of the same pinned upstream commit without applying
the feature patch. Use the same x86 options and submodule pins. Install that
clean baseline DLL and its matching PDB in another isolated copy of the same
runtime and executable. Keep feature saves separate from baseline saves.

For baseline regression mode, copy `diagnostics/regressions.map` as
`spawnmap.ini`, use clean offline settings, and run:

```bat
set PROBE_BASELINE=1
echo regressions>native-probe-mode.txt
Syringe.exe game.exe -i=Vinifera.dll -i=NativeProbe.dll --args="-SPAWN -CD."
set PROBE_BASELINE=
```

The baseline-specific assertions expect the new key to be ignored. For the
direct baseline capture controls, copy `diagnostics/baseline-core.map` as
`spawnmap.ini`, retain clean offline settings, set the mode to
`baseline_control`, and launch with the same command. This map adds the
second forbidden-key vehicle needed for the proximity control. The recorded
baseline control run passed 13 assertions covering forced capture and
proximity theft. Remove the probe when these local checks are finished.
