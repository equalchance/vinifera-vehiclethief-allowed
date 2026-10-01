// Isolated engine diagnostics. Include after the probe's shared observers/helpers.
// Controlled positions exercise native execution; they do not simulate pathfinding.
#pragma once
#include "building.h"
#include "buildingtype.h"
#include "cell.h"
#include "map.h"
#include "scenario.h"
#include "house.h"
#include <climits>
#include <vector>

namespace NativeRegression {
using ObjAction = ActionType(__thiscall*)(InfantryClass*, const ObjectClass*, bool);
static ActionType action(InfantryClass* infantry, const ObjectClass* target)
{
    // The upstream TSpp object/cell bindings are swapped. This signature and
    // native ret 8 were verified against the supported executable.
    return reinterpret_cast<ObjAction>(0x004D6FB0)(infantry, target, false);
}
static void assign_capture(InfantryClass* infantry, TechnoClass* target)
{
    infantry->Assign_Destination(target);
    infantry->Assign_Target(target);
    infantry->CurrentMission = MISSION_CAPTURE;
    infantry->MissionQueue = MISSION_NONE;
}
static void clear_order(InfantryClass* infantry)
{
    infantry->Assign_Destination(nullptr);
    infantry->Assign_Target(nullptr);
    infantry->CurrentMission = MISSION_GUARD;
    infantry->MissionQueue = MISSION_NONE;
}
static bool consumed(InfantryClass* infantry)
{
    return !present(infantry) || !infantry->IsActive || infantry->IsInLimbo;
}
}

static void Run_Native_Regression_Cases(FILE* log, void (*verify)(const char*, bool))
{
    using namespace NativeRegression;
    bool baseline = GetEnvironmentVariableA("PROBE_BASELINE", nullptr, 0) != 0;
    auto hijack = find<InfantryClass>(0x007E2300, "VTHIJACK");
    auto thief = find<InfantryClass>(0x007E2300, "VTTHIEF");
    auto spy = find<InfantryClass>(0x007E2300, "CHAMSPY");
    auto legacy = find<UnitClass>(0x007B3458, "VTLEGACY");
    auto train = find<UnitClass>(0x007B3458, "VTTRAIN");
    auto harvest = find<UnitClass>(0x007B3458, "HARV");
    auto limited = find<UnitClass>(0x007B3458, "VTLIMIT");
    auto radar = find<BuildingClass>(0x007E4708, "NARADR");
    begin_case("REGRESSION_SETUP");
    verify("regression objects present", hijack && thief && spy && legacy && train && harvest && limited && radar);
    if (!hijack || !thief || !spy || !legacy || !train || !harvest || !limited || !radar) return;

    begin_case("N08");
    for (const auto* name : {"VTAIRDEFAULT", "VTAIRYES", "VTAIRNO"}) {
        auto aircraft = find<AircraftClass>(0x007E4058, name);
        verify("aircraft matrix fixture exists", aircraft != nullptr);
        if (!aircraft) continue;
        int old_height = aircraft->Get_Height_AGL();
        aircraft->Set_Height_AGL(0);
        verify("landed aircraft classified as vehicle", aircraft->Considered_Vehicle());
        verify("landed aircraft action matches permission", (action(hijack, aircraft) == ACTION_CAPTURE) == (baseline || std::strcmp(name, "VTAIRNO") != 0));
        aircraft->Set_Height_AGL(512);
        verify("airborne aircraft excluded by native classification", !aircraft->Considered_Vehicle());
        verify("airborne aircraft capture action rejected", action(hijack, aircraft) != ACTION_CAPTURE);
        auto owner = aircraft->House;
        watched = aircraft;
        int old_entries = entries;
        assign_capture(hijack, aircraft);
        hijack->Per_Cell_Process(PCP_END);
        verify("airborne forced order preserves owner and infantry", aircraft->House == owner && present(hijack) && hijack->IsActive && !hijack->IsInLimbo);
        verify("airborne forced order fires no entry event", entries == old_entries);
        clear_order(hijack);
        aircraft->Set_Height_AGL(old_height);
    }

    begin_case("N10");
    verify("legacy NonVehicle configured", legacy->Class->IsNonVehicle);
    verify("legacy unit not considered vehicle", !legacy->Considered_Vehicle());
    verify("legacy VehicleThief capture action unavailable", action(hijack, legacy) != ACTION_CAPTURE);
    auto legacy_owner = legacy->House;
    watched = legacy;
    int old_entries = entries;
    assign_capture(hijack, legacy);
    hijack->Per_Cell_Process(PCP_END);
    verify("legacy forced capture preserves owner and infantry", legacy->House == legacy_owner && present(hijack) && hijack->IsActive);
    verify("legacy forced capture does not enter", entries == old_entries);
    clear_order(hijack);
    legacy->Class->IsNonVehicle = false;
    verify("removing legacy classification permits VehicleThief action", legacy->Considered_Vehicle() && action(hijack, legacy) == ACTION_CAPTURE);
    legacy->Class->IsNonVehicle = true;

    begin_case("N18");
    verify("train flag configured", train->Class->IsTrain);
    verify("train VehicleThief action rejected", action(hijack, train) != ACTION_CAPTURE);
    train->Class->IsTrain = false;
    verify("non-train positive action control", action(hijack, train) == ACTION_CAPTURE);
    train->Class->IsTrain = true;

    begin_case("N17");
    auto original_owner = harvest->House;
    harvest->House = hijack->House;
    verify("own vehicle cannot be hijacked", action(hijack, harvest) != ACTION_CAPTURE);
    harvest->House = original_owner;
    unsigned original_allies = hijack->House->Allies;
    using IsAlly = bool(__thiscall*)(const HouseClass*, const HouseClass*);
    auto is_ally = reinterpret_cast<IsAlly>(0x004BDA20);
    verify("alliance control uses distinct houses", hijack->House != harvest->House);
    hijack->House->Allies = original_allies | (1u << harvest->House->HeapID);
    verify("distinct allied vehicle cannot be hijacked", is_ally(hijack->House, harvest->House) && action(hijack, harvest) != ACTION_CAPTURE);
    hijack->House->Allies = original_allies;
    verify("restoring hostile alliance restores capture action", action(hijack, harvest) == ACTION_CAPTURE);

    begin_case("N19");
    auto scenario = *reinterpret_cast<ScenarioClass**>(0x007E2438);
    verify("scenario exists", scenario != nullptr);
    if (scenario) {
        bool old_truce = scenario->Special.IsHarvesterImmune;
        scenario->Special.IsHarvesterImmune = false;
        verify("harvester action allowed without truce", action(hijack, harvest) == ACTION_CAPTURE);
        scenario->Special.IsHarvesterImmune = true;
        verify("harvester action forbidden with truce", action(hijack, harvest) != ACTION_CAPTURE);
        scenario->Special.IsHarvesterImmune = old_truce;
    }

    begin_case("N06_SCAN");
    auto allowed = find<UnitClass>(0x007B3458, "VTDEFAULT");
    auto denied = find<UnitClass>(0x007B3458, "VTNO");
    auto both = find<InfantryClass>(0x007E2300, "VTBOTH");
    auto armed = find<InfantryClass>(0x007E2300, "VTARMED");
    verify("automatic acquisition fixture objects exist", allowed && denied && both && armed);
    if (allowed && denied && both && armed) {
        struct State { TechnoClass* object; MissionType mission; bool locked, discovered, limbo; };
        std::vector<State> states;
        auto remember = [&states](TechnoClass* p) {
            states.push_back({p, p->CurrentMission, p->IsLocked, p->IsDiscoveredByPlayer, p->IsInLimbo});
        };
        for (auto p : {allowed, denied, harvest}) {
            remember(p);
            p->CurrentMission = MISSION_GUARD;
            p->IsLocked = true;
            p->IsDiscoveredByPlayer = true;
        }
        using Evaluate = bool(__thiscall*)(const TechnoClass*, ThreatType, int, int, const TechnoClass*, int&, int, const Coord&);
        auto evaluate = reinterpret_cast<Evaluate>(0x0062D0F0);
        auto candidate = [&](InfantryClass* infantry, TechnoClass* target) {
            int value = 0;
            return evaluate(infantry, static_cast<ThreatType>(THREAT_VEHICLES | THREAT_BASE_DEFENSE), (1 << RTTI_UNIT) | 2, -1, target, value, -1, infantry->Position);
        };
        for (auto infantry : {hijack, thief, both}) {
            verify("unarmed theft scan accepts allowed vehicle", candidate(infantry, allowed));
            verify("unarmed theft scan permission matches build", candidate(infantry, denied) == baseline);
        }
        verify("armed infantry ordinary attack scan accepts forbidden vehicle", candidate(armed, denied));
        if (scenario) {
            bool truce = scenario->Special.IsHarvesterImmune;
            scenario->Special.IsHarvesterImmune = false;
            verify("automatic scan accepts harvester without truce", candidate(hijack, harvest));
            scenario->Special.IsHarvesterImmune = true;
            verify("automatic scan rejects harvester with truce", !candidate(hijack, harvest));
            scenario->Special.IsHarvesterImmune = truce;
        }
        // Restrict the live global scan to the two candidate vehicles. This is
        // a real Greatest_Threat scan with no existing navigation shortcut.
        auto hide = [&](auto& list) {
            for (int i=0; i<list.Count(); ++i) {
                auto p = static_cast<TechnoClass*>(list.data()[i]);
                if (p && p != allowed && p != denied && p != hijack && p != thief && p != both && p != armed) {
                    remember(p); p->IsInLimbo = true;
                }
            }
        };
        hide(objects<UnitClass>(0x007B3458));
        hide(objects<AircraftClass>(0x007E4058));
        hide(objects<BuildingClass>(0x007E4708));
        clear_order(hijack); clear_order(thief); clear_order(both);
        auto scan_method = static_cast<ThreatType>(THREAT_VEHICLES | THREAT_BASE_DEFENSE);
        auto selected = hijack->Greatest_Threat(scan_method, hijack->Position);
        verify("automatic VehicleThief scan selects a legal candidate", selected == allowed || (baseline && selected == denied));
        allowed->IsInLimbo = true;
        auto sole_target = hijack->Greatest_Threat(scan_method, hijack->Position);
        verify("automatic VehicleThief scan refuses sole forbidden candidate", baseline ? sole_target == denied : sole_target == nullptr);
        verify("Thief-only retains stock lack of automatic acquisition", thief->Greatest_Threat(scan_method, thief->Position) == nullptr);
        // Restore object state before the execution regressions below.
        for (auto it=states.rbegin(); it!=states.rend(); ++it) {
            it->object->CurrentMission = it->mission;
            it->object->IsLocked = it->locked;
            it->object->IsDiscoveredByPlayer = it->discovered;
            it->object->IsInLimbo = it->limbo;
        }
    }

    begin_case("N20");
    using CanDeploy = bool(__thiscall*)(const TechnoClass*);
    auto can_deploy = reinterpret_cast<CanDeploy>(0x00632070);
    using UnitAction = ActionType(__thiscall*)(UnitClass*, const ObjectClass*, bool);
    auto unit_action = reinterpret_cast<UnitAction>(0x00655F90);
    verify("limited deployable fixture configured", limited->Class->DeploysInto != nullptr && limited->Class->BuildLimit == 1);
    verify("unhijacked limited vehicle can deploy", limited->EnteredByInfType == INFANTRY_NONE && can_deploy(limited));
    auto new_owner = hijack->House;
    auto infantry_type = hijack->Class->HeapID;
    hijack->Position = limited->Position;
    watched = limited;
    old_entries = entries;
    assign_capture(hijack, limited);
    hijack->Per_Cell_Process(PCP_END);
    verify("limited allowed vehicle captured", limited->House == new_owner);
    verify("limited capture consumes infantry", consumed(hijack));
    verify("limited capture entry positive control", entries > old_entries);
    verify("hijacker type recorded", limited->EnteredByInfType == infantry_type);
    verify("hijacked build-limited vehicle deployment rejected", !can_deploy(limited));
    // Native self-action requires exactly one selected object (0x007E4868).
    // Select through the real virtual routine rather than changing that count.
    // The capture happened before a visibility-update frame. Native Foot action
    // converts self-action into movement over shroud; reveal through Map's API.
    reinterpret_cast<void(__thiscall*)(MapClass*)>(0x0051E0A0)(reinterpret_cast<MapClass*>(0x00748348));
    static_cast<ObjectClass*>(limited)->Select();
    verify("deployment click has one selected native vehicle", limited->IsSelected && *reinterpret_cast<int*>(0x007E4868) == 1);
    verify("hijacked build-limited vehicle rejects player self action", unit_action(limited, limited, false) == ACTION_NO_DEPLOY);
    using UnitClick = bool(__thiscall*)(UnitClass*, ActionType, ObjectClass*, bool);
    auto click = reinterpret_cast<UnitClick>(0x00650330);
    watched_deployment = limited;
    int before_orders = deployment_orders;
    click(limited, ACTION_SELF, limited, false);
    verify("hijacked build-limited deployment click emits no unload mission", deployment_orders == before_orders);
    int limit = limited->Class->BuildLimit;
    limited->Class->BuildLimit = INT_MAX;
    verify("unlimited deployment positive control", can_deploy(limited));
    verify("unlimited vehicle player self action positive control", unit_action(limited, limited, false) == ACTION_SELF);
    click(limited, ACTION_SELF, limited, false);
    verify("unlimited deployment click submits native unload mission", deployment_orders == before_orders + 1);
    watched_deployment = nullptr;
    static_cast<ObjectClass*>(limited)->Unselect();
    limited->Class->BuildLimit = limit;

    begin_case("N10_THIEF");
    // The stock proximity route deliberately lacks the NonVehicle check.
    using Theft = bool(__thiscall*)(InfantryClass*);
    auto theft = reinterpret_cast<Theft>(0x004D8390);
    auto thief_owner = thief->House;
    thief->Position = legacy->Position;
    thief->Assign_Destination(legacy);
    watched = legacy;
    old_entries = entries;
    verify("Thief legacy proximity capture completes", theft(thief));
    verify("Thief legacy owner transfers", legacy->House == thief_owner);
    verify("Thief legacy infantry consumed", consumed(thief));
    // VTLEGACY has no tag, so event observation is not asserted for this case.

    begin_case("N16");
    verify("infiltrator agent and theft flags configured", spy->Class->IsAgent && spy->Class->IsVehicleThief && spy->Class->IsThief);
    auto extension = *reinterpret_cast<unsigned char**>(reinterpret_cast<unsigned char*>(radar->Class) + 0x10);
    verify("building type extension exists", extension != nullptr);
    if (extension && !baseline) verify("building permission no loaded from INI", extension[844] == 0);
    verify("building infiltration action retained", action(spy, radar) == ACTION_CAPTURE);
    auto radar_owner = radar->House;
    auto spy_owner = spy->House;
    unsigned before_spied = radar->SpiedBy;
    // Find an actually occupied building cell rather than assuming its center.
    auto position = radar->Position;
    using GetCell = CellClass&(__thiscall*)(MapClass*, const Cell&);
    using CellBuilding = BuildingClass*(__thiscall*)(const CellClass*);
    auto get_cell = reinterpret_cast<GetCell>(0x0050F280);
    auto cell_building = reinterpret_cast<CellBuilding>(0x00452160);
    bool occupied = false;
    for (int dx=-2; dx<=2 && !occupied; ++dx) {
        for (int dy=-2; dy<=2 && !occupied; ++dy) {
            Cell cell {static_cast<short>(position.X/256+dx), static_cast<short>(position.Y/256+dy)};
            if (cell_building(&get_cell(reinterpret_cast<MapClass*>(0x00748348), cell)) == radar) {
                spy->Position = Coord {cell.X*256+128, cell.Y*256+128, position.Z};
                occupied = true;
            }
        }
    }
    verify("spy enters a native occupied building cell", occupied);
    if (occupied) {
        watched = radar;
        assign_capture(spy, radar);
        spy->Per_Cell_Process(PCP_END);
        verify("infiltration spy consumed", consumed(spy));
        verify("infiltration building owner unchanged", radar->House == radar_owner);
        verify("infiltration updates native SpiedBy for the infiltrating house", radar->SpiedBy == (before_spied | (1u << spy_owner->HeapID)));
    }
    watched = nullptr;
    std::fprintf(log, "Native regression scope: controlled engine calls; deployment eligibility, not transformation.\n");
}
