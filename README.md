# VehicleThief.Allowed

Adds `VehicleThief.Allowed` to vehicle and aircraft types. It defaults to `yes`;
set it to `no` to block hijacking without using `NonVehicle`.

```ini
[ExampleDrone]
NonVehicle=no
VehicleThief.Allowed=no
```

This applies to `VehicleThief=yes` capture orders and `Thief=yes` proximity theft.
`Thief` keeps its vehicle-only behavior. The flag adds a veto; `yes` preserves
existing restrictions for `NonVehicle`, trains, allies, and aircraft landing.
Repair still depends on the repair weapon and normal game rules. Keeping
`NonVehicle=yes` keeps its existing side effects.

For a mod using another Vinifera branch, apply the source patch and rebuild.
[Build steps](docs/BUILDING.md) and [installation notes](docs/INSTALLING.md)
cover that. The supplied binary targets upstream
`a02a90f816328ac59f94b455982d22979fed1de1`; dependency and runtime hashes are in
`pins.json`.

[Downloads](https://github.com/equalchance/vinifera-vehiclethief-allowed/releases/tag/experimental-1)
include the DLL with matching symbols, patched source, and test fixtures.
[Testing notes](docs/TESTING.md) list what passed and the remaining limits.
Retest on the Vinifera branch you are porting to.

Original request: [#634](https://github.com/Vinifera-Developers/Vinifera/issues/634).
