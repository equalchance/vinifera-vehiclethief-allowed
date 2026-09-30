Put the `VehicleThief.Allowed` patch and test build here so it's easier to try.

It defaults to `yes`. Setting it to `no` blocks hijacking without needing
`NonVehicle=yes`, so a drone can still use normal vehicle repair and carryall
handling.

The in-game checks passed, including multiplayer and save/load. The deeper
execution cases and the private TI build still need testing; see
[the test notes](https://github.com/equalchance/vinifera-vehiclethief-allowed/blob/main/docs/TESTING.md).

`Vinifera-VehicleThief-Allowed-test-kit.zip` has the patch and fixtures.
`Vinifera-VehicleThief-Allowed-source.zip` has the patched source and pinned
submodule sources. Check `SHA256SUMS.txt` for the download hashes.
The DLL/PDB are pending a final check after local build paths are removed.
