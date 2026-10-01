Adds `VehicleThief.Allowed` to vehicle and aircraft types. It defaults to `yes`;
set it to `no` to block hijacking independently of `NonVehicle`.

The theft checks and cold save/load passed. Normal gameplay and multiplayer
checks were also reported as passing. [Test notes](https://github.com/equalchance/vinifera-vehiclethief-allowed/blob/main/docs/TESTING.md)
list the coverage and remaining limits. Retest on the Vinifera branch you are
porting to.

- `Vinifera-VehicleThief-Allowed-binaries.zip`: DLL and matching symbols.
- `Vinifera-VehicleThief-Allowed-source.zip`: patched source and pinned dependencies.
- `Vinifera-VehicleThief-Allowed-test-kit.zip`: fixtures and review checks.

Use `SHA256SUMS.txt` to verify downloads. For another Vinifera branch, apply the
source patch and rebuild against that branch's runtime.
