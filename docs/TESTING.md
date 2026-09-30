# Test notes

The in-game checks passed for:

- Hijacking allowed vehicles and landed aircraft, including explicit `yes`.
- Blocking the hijack cursor on `no` vehicles and aircraft.
- Keeping allowed/blocked targeting after save/load.
- Repairing the damaged drone with a vehicle repair weapon.
- Picking up and unloading that drone with a carryall.
- Boarding a friendly transport with the hijacker.
- Attacking a blocked theft target with an armed infantry unit.
- Following an allowed moving target with VTHIJACK.
- Multiplayer hijacking, repair, carryall handling, and save/load.
- Rejecting a save from the clean baseline after the save-version change.

The clean baseline can hijack the vehicles and aircraft using the new `no` key,
because it doesn't read that key. Train hijacking remains blocked in the feature.
The source review found no confirmed implementation defects. Build, C++ branch,
hook-byte/register, type-layout, and DLL/PDB checks passed too.

Still unverified: forced capture execution, Thief-only proximity theft,
automatic acquisition, queued orders, and restored forbidden destinations.
The detailed regression matrix also has incomplete cases for airborne aircraft,
legacy classification, harvester truce, build-limit deployment, and infiltration.
Cursor rejection doesn't exercise an already assigned forbidden theft order.
No private TI build has been tested.

## Try the fixture

Use a separate installation with the matched executable/launcher and Vinifera
runtime files. Build the patched source and back up the installation's DLL/PDB, then install
the matching pair from that build.
Restore the backups to roll back. Close the game before changing binaries.

For the offline fixture, copy `test-kit/vehiclethief-offline.map` to
`spawnmap.ini` and `test-kit/spawn-offline.ini` to `spawn.ini`, then launch
`LaunchVinifera.exe -SPAWN -CD.`. Use GDI. The map supplies its test types through
existing stock assets; `test-kit/vehiclethief-rules.ini` lists those definitions.

Enemy tooltips may hide the test IDs. Use `test-kit/POSITIONS.svg` to identify
the starting targets. The upper buggy diagonal is VTDEFAULT, VTYES, VTNO in
that order; the Orcas follow the same omitted/yes/no pattern.

For TS Client, `client/MPMaps.ini` is the fixture catalog. Merge its numbered
map/mode entries into the test installation's catalog, preserving contiguous
indices, and place the two maps under `Maps/VehicleThief/offline.map` and
`Maps/VehicleThief/online.map`. Copy `client/VTA.ini` to `INI/Map Code/VTA.ini`.
Catalog control names match the supplied TS Client layout; adapt them if a mod's
lobby differs. Use Offline launch check in Skirmish. The online map requires
two human players, GDI/Gold at position 1 and Nod/Red at position 2, with no teams.

The client save helper in `client/Support/Sync-MPSaves.ps1` handles the numbered
SAVEGAME/SVGM filename mismatch. It keeps the originals and creates client-named
copies. Place it under `Support`, and put the two CMD files beside the test
installation's `TiberianSun.exe`. Copy `client/vehiclethief-client.json` there
too so the helper recognizes the isolated copy. Exit the game and client, then
use `Refresh MP saves.cmd` before reopening the client. Enable
`CreateSavedGamesDirectory=true` in the client's definitions. The helper changes
filenames for client discovery; it doesn't convert saves between builds.

For a private TI branch, apply the source patch and build with that branch's
matched runtime. Retest theft, drone repair, carryall, save/load, and multiplayer
there before using it in the mod.
