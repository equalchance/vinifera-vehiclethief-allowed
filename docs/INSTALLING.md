# Install

Use a separate test installation. The binary ZIP contains `Vinifera.dll` and
its matching `Vinifera.pdb`; it does not contain the game or complete runtime.
Check the executable and launcher SHA256 values in `pins.json`.
This build uses Vinifera's spawner with the matched no-spawner executable.
For a different branch or executable, apply the source patch and rebuild.

Close the game and back up its existing DLL and symbols. Copy the supplied
pair beside `game.exe`, keeping the PDB filename. Symbols are optional for
playing but help diagnose crashes.

The save format changes: same-build saves work; older saves are rejected
without migration. Keep other builds' saves separate. To roll back, close the
game and restore the backed-up binaries and their saves.

To try the offline fixture, copy `test-kit/vehiclethief-offline.map` as
`spawnmap.ini` and `test-kit/spawn-offline.ini` as `spawn.ini`, then run
`LaunchVinifera.exe -SPAWN -CD.`. Use GDI; `test-kit/POSITIONS.svg` identifies
the targets. The fixture uses existing stock assets.
