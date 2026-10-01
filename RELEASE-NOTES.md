Put the `VehicleThief.Allowed` patch and test build here so it's easier to try.

It defaults to `yes`. Setting it to `no` blocks hijacking without needing
`NonVehicle=yes`, so a drone can still use normal vehicle repair and carryall
handling.

The native probes passed 389 assertions across eight runs with zero failures,
including forced theft orders and cold save/load. Normal gameplay and
multiplayer checks were reported as passing too. Broader cases and the private
TI build remain open; see
[the test notes](https://github.com/equalchance/vinifera-vehiclethief-allowed/blob/main/docs/TESTING.md).

`Vinifera-VehicleThief-Allowed-test-kit.zip` has the patch and fixtures.
`Vinifera-VehicleThief-Allowed-source.zip` has the patched source and pinned
submodule sources. `Vinifera-VehicleThief-Allowed-binaries.zip` has the DLL and
matching debugging symbols. Check `SHA256SUMS.txt` for the download hashes.
