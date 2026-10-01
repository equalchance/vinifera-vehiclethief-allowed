# Testing

Tested on upstream `a02a90f816328ac59f94b455982d22979fed1de1`, built with
MSVC 14.44.35207 x86, Ninja, `RelWithDebInfo`, and `PREVIEW=ON`.
The matched executable and dependency hashes are in `pins.json`.

Passed:

- Allowed/blocked VehicleThief theft on vehicles and landed aircraft; Thief vehicle proximity theft.
- Forced forbidden orders, candidate selection, queued orders, and cold save/load.
- Controlled airborne state, legacy classification, harvester truce, deployment-order restrictions, and infiltration.
- Manual gameplay reports: drone repair, carryall handling, boarding, and ordinary attacks.
- Reported multiplayer checks: hijacking, repair, carryall handling, and save/load.

Automated checks passed 389 repeated assertions across eight runs with zero
failures. [Native results](NATIVE-CHECKS.md) link the receipts.

Multi-frame autonomous pursuit, normal flight, completed allowed deployment,
and detailed concurrent/queued multiplayer instrumentation remain unverified.
Retest on the Vinifera branch you are porting to.
