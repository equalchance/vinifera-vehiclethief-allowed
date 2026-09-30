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

In-game checks passed for allowed and blocked hijacking, drone repair, carryall
pickup/unload, passenger boarding, normal weapon attacks, multiplayer, and
save/load. The deeper execution cases still need testing; [the test notes](docs/TESTING.md)
list them. I haven't tested the private TI build.

[Downloads](https://github.com/equalchance/vinifera-vehiclethief-allowed/releases/tag/experimental-1)
have the test kit and patched source. Use the patch if you're
working on a different Vinifera branch. The build is based on
`a02a90f816328ac59f94b455982d22979fed1de1`; [build instructions](docs/BUILDING.md)
include the exact dependency versions.

Binaries are not included in this upload. The DLL/PDB need a final check after
removing local build paths; build the source to try the feature.

This is here for review and testing before a Vinifera PR. The original request
is [#634](https://github.com/Vinifera-Developers/Vinifera/issues/634).
