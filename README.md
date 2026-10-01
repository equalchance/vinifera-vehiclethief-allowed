# VehicleThief.Allowed

This adds `VehicleThief.Allowed` to vehicle and aircraft types, so hijacking
can be blocked without using `NonVehicle`.
It defaults to `yes`; set it to `no` to block hijacking.

```ini
[ExampleDrone]
NonVehicle=no
VehicleThief.Allowed=no
```

That keeps the drone classified as a vehicle. Repair still depends on the repair
weapon and the usual game rules. Keeping `NonVehicle=yes` keeps its side effects.
The new flag is an extra restriction, so `yes` doesn't bypass the old rules for
allies, trains, or airborne aircraft. `Thief=yes` keeps its existing vehicle-only
proximity behavior.

Native probes passed 389 assertions across eight runs, including both theft
paths, forced forbidden orders, automatic candidate selection, queued orders,
regressions, and cold save/load. Those are repeated assertions across runs,
not 389 distinct scenarios. Normal gameplay and multiplayer checks were also
reported as passing; [the test notes](docs/TESTING.md) keep their scope and
remaining limits explicit. I haven't tested the private TI build.
The [native check instructions](docs/NATIVE-CHECKS.md) include the probe source
and steps for repeating execution and saved-order checks.

[Downloads](https://github.com/equalchance/vinifera-vehiclethief-allowed/releases/tag/experimental-1)
have the DLL/PDB, test kit, and patched source. Use the patch if you're
working on a different Vinifera branch. The build is based on
`a02a90f816328ac59f94b455982d22979fed1de1`; [build instructions](docs/BUILDING.md)
include the exact dependency versions.

The binary download includes `Vinifera.dll` and its matching `Vinifera.pdb`.
Use a separate test install with the matched runtime; [installation notes](docs/INSTALLING.md)
cover the files and rollback.

This is here for Vinifera review and testing. The original request
is [#634](https://github.com/Vinifera-Developers/Vinifera/issues/634).
