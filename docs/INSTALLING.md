# Install the test DLL

Use a separate test installation with the runtime matched in `pins.json`.
The binary ZIP contains `Vinifera.dll` and `Vinifera.pdb`; it doesn't include
Tiberian Sun, the launcher, or the remaining Vinifera runtime files.

Close the game, back up its existing DLL and symbols, then copy both files from
the binary ZIP beside `game.exe`. Keep the filename `Vinifera.pdb`: it matches
the debug record embedded in the DLL. The PDB helps diagnose crashes; the game
runs without it. Restore the backups to roll back.

This build uses Vinifera's spawner and the matched no-spawner executable.
Check the executable and launcher hashes in `pins.json` before installing.
For a different branch or executable, apply the source patch and rebuild.

Keep saves from other builds separate. Same-build save/load works; the changed
save format rejects older saves without migration. Fixture setup is in
[TESTING.md](TESTING.md). Private TI-build acceptance remains pending.
