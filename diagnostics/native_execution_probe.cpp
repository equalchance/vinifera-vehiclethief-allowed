// Local test instrumentation; never load this DLL in multiplayer or a normal install.
// Calls the pinned engine on its main thread. Vinifera.dll is kept byte-identical.
#include "always.h"
#include "syringe.h"
#include "infantry.h"
#include "infantrytype.h"
#include "unit.h"
#include "unittype.h"
#include "aircraft.h"
#include "aircrafttype.h"
#include <cstdio>
#include <cstring>
#include <initializer_list>

// Header-only TSpp container checks need local assertion plumbing in this
// standalone probe; terminate rather than letting an invalid index proceed.
bool TSPP_IgnoreAllAsserts = false;
bool TSPP_ExitOnAssert = true;
void Emergency_Exit(int code) { ExitProcess(static_cast<UINT>(code)); }
static void __cdecl Probe_Assertion(TSPPAssertType, const char*, const char*, int, const char*, volatile bool*, volatile bool*, volatile bool*, const char*, ...) { ExitProcess(98); }
void (*TSPP_Assertion_Handler_Ptr)(TSPPAssertType, const char*, const char*, int, const char*, volatile bool*, volatile bool*, volatile bool*, const char*, ...) = Probe_Assertion;

static FILE* output;
static bool done;
static int failures, checks, entries;
static ObjectClass* watched;
static const TechnoClass* watched_deployment;
static int deployment_orders;
static const char* current_case = "FIXTURE";
static void begin_case(const char* id) { current_case = id; }

// The campaign fixture otherwise waits in the briefing dialog before Main_Loop.
// Skip that UI-only routine at its verified six-byte entry instruction. The
// standalone SyringeEx explicitly supports applying ESP changes in callbacks.
DEFINE_HOOK(0x005C0230, NativeProbe_SkipBriefing, 6)
{
    auto resume = R->Stack<DWORD>(0);
    R->ESP(R->ESP() + 4);
    return resume;
}

template<class T> static DynamicVectorClass<T*>& objects(unsigned address)
{
    return *reinterpret_cast<DynamicVectorClass<T*>*>(address);
}
template<class T> static T* find(unsigned address, const char* name)
{
    auto& list = objects<T>(address);
    for (int i = 0; i < list.Count(); ++i) {
        auto p = list.data()[i];
        if (p && !std::strcmp(p->Class->Name(), name)) return p;
    }
    return nullptr;
}
static bool present(InfantryClass* p)
{
    auto& list = objects<InfantryClass>(0x007E2300);
    for (int i = 0; i < list.Count(); ++i) if (list.data()[i] == p) return true;
    return false;
}
static void check(const char* name, bool value)
{
    ++checks;
    failures += !value;
    std::fprintf(output, "%s CASE=%s %s\n", value ? "PASS" : "FAIL", current_case, name);
    std::fflush(output);
}
#if __has_include("native_regression_cases.h")
#include "native_regression_cases.h"
#endif
// Observe the real entry event, including a positive control. No behavior change.
DEFINE_HOOK(0x0061E880, NativeProbe_EntryEvent, 5)
{
    if (output && watched && R->Stack<int>(4) == TEVENT_PLAYER_ENTERED
        && R->ECX<void*>() == watched->Tag) ++entries;
    return 0;
}
// Observer only. Native Player_Assign_Mission entry: ECX=this, stack4=mission.
DEFINE_HOOK(0x00631650, NativeProbe_DeploymentOrder, 5)
{
    if (watched_deployment && R->ECX<TechnoClass*>() == watched_deployment && R->Stack<MissionType>(4) == MISSION_UNLOAD) ++deployment_orders;
    return 0;
}
DEFINE_HOOK(0x00508A40, NativeProbe_MainLoop, 5)
{
    char mode[32] = "execution";
    if (auto file = std::fopen("native-probe-mode.txt", "r")) { std::fscanf(file, "%31s", mode); std::fclose(file); }
    if (done || (!std::strcmp(mode,"execution") && *reinterpret_cast<long*>(0x007E4924) < 5)) return 0;
    done = true;
    output = std::fopen("native-execution.log", "w");
    if (!output) return 0;
    std::fprintf(output, "Native main-thread execution probe; frame=%ld\n", *reinterpret_cast<long*>(0x007E4924));
    if (!std::strcmp(mode, "regressions")) {
#if __has_include("native_regression_cases.h")
        begin_case("REGRESSIONS");
        Run_Native_Regression_Cases(output, check);
#else
        check("regression header available", false);
#endif
        std::fprintf(output, "RESULT checks=%d failures=%d entry_events=%d\n", checks, failures, entries);
        std::fclose(output); ExitProcess(failures ? 1 : 0); return 0;
    }
    auto hijack = find<InfantryClass>(0x007E2300, "VTHIJACK");
    auto thief = find<InfantryClass>(0x007E2300, "VTTHIEF");
    auto both = find<InfantryClass>(0x007E2300, "VTBOTH");
    auto denied = find<UnitClass>(0x007B3458, "VTNO");
    auto allowed = find<UnitClass>(0x007B3458, "VTDEFAULT");
    auto aircraft = find<AircraftClass>(0x007E4058, "VTAIRDEFAULT");
    check("fixture objects present", hijack && thief && both && denied && allowed && aircraft);
    if (failures) { std::fclose(output); output = nullptr; return 0; }
    std::fprintf(output, "class flags: hijack=%d/%d thief=%d/%d both=%d/%d\n",
        hijack->Class->IsVehicleThief, hijack->Class->IsThief,
        thief->Class->IsVehicleThief, thief->Class->IsThief,
        both->Class->IsVehicleThief, both->Class->IsThief);
    if (!std::strcmp(mode, "baseline_control")) {
        begin_case("BASELINE_CONTROL");
        for (auto infantry : {hijack, both, thief}) {
            TechnoClass* target = nullptr;
            if (infantry == both) target = find<AircraftClass>(0x007E4058, "VTAIRNO");
            else {
                auto& units = objects<UnitClass>(0x007B3458);
                for (int i=0;i<units.Count();++i) {
                    auto unit = units.data()[i];
                    if (unit && !std::strcmp(unit->Class->Name(), "VTNO") && unit->House != infantry->House) { target = unit; break; }
                }
            }
            check("baseline forbidden-key target exists", target != nullptr);
            if (!target) continue;
            auto new_owner = infantry->House; watched = target; auto before = entries;
            infantry->Position = target->Position;
            infantry->Assign_Destination(target); infantry->Assign_Target(target);
            infantry->CurrentMission = MISSION_CAPTURE; infantry->MissionQueue = MISSION_NONE;
            if (infantry == thief) reinterpret_cast<bool(__thiscall*)(InfantryClass*)>(0x004D8390)(infantry);
            else infantry->Per_Cell_Process(PCP_END);
            check("baseline ignores forbidden key and transfers ownership", target->House == new_owner);
            check("baseline allowed capture consumes infantry", !present(infantry) || !infantry->IsActive || infantry->IsInLimbo);
            check("baseline allowed capture emits entry event", entries > before);
        }
        std::fprintf(output,"RESULT checks=%d failures=%d entry_events=%d\n",checks,failures,entries);
        std::fclose(output); ExitProcess(failures ? 1 : 0); return 0;
    }
    if (!std::strcmp(mode, "save") || !std::strcmp(mode, "load")) {
        begin_case("RESTORED_DESTINATION");
        auto denied_air = find<AircraftClass>(0x007E4058, "VTAIRNO");
        if (!std::strcmp(mode, "save")) {
            for (auto infantry : {hijack, thief, both}) {
                auto target = infantry == both ? static_cast<TechnoClass*>(denied_air) : static_cast<TechnoClass*>(denied);
                infantry->Assign_Destination(target); infantry->Assign_Target(target);
                infantry->CurrentMission = MISSION_CAPTURE; infantry->MissionQueue = MISSION_NONE;
                check("forbidden destination assigned before saving", infantry->NavCom == target && infantry->TarCom == target);
            }
            auto save = reinterpret_cast<bool(__fastcall*)(const char*, const char*, bool)>(0x005D4FE0);
            check("native Save_Game writes assigned forbidden destination", save("NativeForbidden.sav", "Native forbidden theft destination", false));
        } else {
            for (auto infantry : {hijack, thief, both}) {
                auto target = infantry == both ? static_cast<TechnoClass*>(denied_air) : static_cast<TechnoClass*>(denied);
                check("cold-load restores forbidden destination identity", infantry->NavCom == target);
                check("cold-load restores forbidden attack target identity", infantry->TarCom == target);
                auto owner = target->House; watched = target; auto before = entries;
                if (infantry == thief) reinterpret_cast<bool(__thiscall*)(InfantryClass*)>(0x004D8390)(infantry);
                else infantry->Per_Cell_Process(PCP_END);
                check("restored forbidden capture leaves ownership unchanged", target->House == owner);
                check("restored forbidden capture leaves infantry active", present(infantry) && infantry->IsActive && !infantry->IsInLimbo);
                check("restored forbidden capture clears destination and target", !infantry->NavCom && !infantry->TarCom);
                check("restored forbidden capture emits no entry trigger", entries == before);
            }
        }
        std::fprintf(output, "RESULT checks=%d failures=%d entry_events=%d\n", checks, failures, entries);
        std::fclose(output); ExitProcess(failures ? 1 : 0); return 0;
    }
    auto yes_unit = find<UnitClass>(0x007B3458, "VTYES");
    begin_case("AUTOMATIC_TARGET");
    auto origin = hijack->Center_Coord();
    auto method = static_cast<ThreatType>(THREAT_VEHICLES | THREAT_RANGE);
    hijack->Assign_Destination(yes_unit);
    check("automatic target shortcut keeps permitted destination", hijack->Greatest_Threat(method, origin, false) == yes_unit);
    hijack->Assign_Destination(denied);
    check("automatic target shortcut excludes forbidden destination", hijack->Greatest_Threat(method, origin, false) != denied);
    hijack->Assign_Destination(nullptr);
    begin_case("QUEUED_ORDER");
    using Queue = void(__thiscall*)(FootClass*, AbstractClass*);
    using AdvanceQueue = void(__thiscall*)(FootClass*);
    auto queue = reinterpret_cast<Queue>(0x004A5480);
    auto advance_queue = reinterpret_cast<AdvanceQueue>(0x004A53D0);
    auto clear_queue = reinterpret_cast<AdvanceQueue>(0x004A5550);
    for (auto infantry : {hijack, thief, both}) {
        clear_queue(infantry); infantry->Assign_Destination(nullptr); queue(infantry, yes_unit);
        advance_queue(infantry);
        check("native navigation queue accepts permitted destination", infantry->NavCom == yes_unit && infantry->NavQueue.Count() == 0);
        infantry->Assign_Destination(nullptr); clear_queue(infantry); queue(infantry, denied);
        check("native navigation queue contains forbidden order", infantry->NavQueue.Count() == 1);
        infantry->CurrentMission = MISSION_CAPTURE; infantry->Assign_Target(denied);
        advance_queue(infantry);
        check("native navigation queue restores forbidden destination", infantry->NavCom == denied && infantry->NavQueue.Count() == 0);
        watched = denied; auto queue_owner = denied->House; auto queue_entries = entries;
        if (infantry == thief) reinterpret_cast<bool(__thiscall*)(InfantryClass*)>(0x004D8390)(infantry);
        else infantry->Per_Cell_Process(PCP_END);
        check("queued forbidden order leaves ownership unchanged", denied->House == queue_owner);
        check("queued forbidden order leaves infantry active", present(infantry) && infantry->IsActive && !infantry->IsInLimbo);
        check("queued forbidden order clears destination and target", !infantry->NavCom && !infantry->TarCom);
        check("queued forbidden order emits no entry event", entries == queue_entries);
        clear_queue(infantry);
    }
    for (ObjectClass* forbidden : {static_cast<ObjectClass*>(denied), static_cast<ObjectClass*>(find<AircraftClass>(0x007E4058, "VTAIRNO"))}) {
      begin_case(forbidden->RTTI == RTTI_AIRCRAFT ? "FORCED_CAPTURE_AIRCRAFT_NO" : "FORCED_CAPTURE_VEHICLE_NO");
      for (auto infantry : {hijack, both}) {
        watched = forbidden;
        int before = entries;
        auto owner = static_cast<TechnoClass*>(forbidden)->House;
        infantry->Assign_Destination(forbidden);
        infantry->Assign_Target(forbidden);
        infantry->CurrentMission = MISSION_CAPTURE;
        infantry->MissionQueue = MISSION_NONE;
        infantry->Per_Cell_Process(PCP_END);
        check("forced capture: infantry remains active in native array", present(infantry) && infantry->IsActive && !infantry->IsInLimbo);
        check("forced capture: owner unchanged", static_cast<TechnoClass*>(forbidden)->House == owner);
        check("forced capture: entry event absent", entries == before);
        check("forced capture: destination cleared", infantry->NavCom == nullptr);
        check("forced capture: attack target cleared", infantry->TarCom == nullptr);
      }
    }
    using Theft = bool(__thiscall*)(InfantryClass*);
    auto theft = reinterpret_cast<Theft>(0x004D8390);
    begin_case("THIEF_PROXIMITY");
    watched = denied;
    int before = entries;
    auto owner = denied->House;
    auto saved_thief_position = thief->Position;
    thief->Position = denied->Position;
    thief->Assign_Destination(denied);
    check("Thief-only forbidden route returns false", !theft(thief));
    check("Thief-only forbidden infantry remains", present(thief));
    check("Thief-only forbidden owner unchanged", denied->House == owner);
    check("Thief-only forbidden entry event absent", entries == before);
    check("Thief-only forbidden destination cleared", thief->NavCom == nullptr);
    thief->Assign_Destination(aircraft);
    owner = aircraft->House;
    check("Thief-only aircraft returns false", !theft(thief));
    check("Thief-only aircraft owner unchanged", aircraft->House == owner);
    check("Thief-only aircraft infantry remains", present(thief));
    thief->Assign_Destination(nullptr);
    // Debugger-equivalent position setup permits the positive proximity branch.
    // This is execution coverage, not evidence of normal pathfinding or occupancy.
    auto old_position = thief->Position;
    thief->Position = allowed->Position;
    watched = allowed;
    before = entries;
    auto new_owner = thief->House;
    thief->Assign_Destination(allowed);
    bool captured = theft(thief);
    check("Thief-only allowed route completes", captured);
    check("Thief-only allowed ownership transfers", allowed->House == new_owner);
    std::fprintf(output, "Thief consumed state: present=%d active=%d limbo=%d strength=%d\n", present(thief), thief->IsActive, thief->IsInLimbo, thief->Strength);
    check("Thief-only allowed infantry consumed", !present(thief) || !thief->IsActive || thief->IsInLimbo);
    check("Thief-only allowed entry event positive control", entries > before);
    if (present(thief)) { thief->Position = old_position; thief->Assign_Destination(nullptr); }
    watched = nullptr;
    begin_case("FORCED_CAPTURE_ALLOWED");
    for (auto infantry : {hijack, both}) {
        ObjectClass* target = infantry == hijack ? static_cast<ObjectClass*>(yes_unit) : static_cast<ObjectClass*>(find<AircraftClass>(0x007E4058, "VTAIRYES"));
        check("allowed forced capture target present", target != nullptr);
        if (!target) continue;
        auto target_techno = static_cast<TechnoClass*>(target);
        watched = target; before = entries; new_owner = infantry->House;
        infantry->Position = target->Position;
        infantry->Assign_Destination(target); infantry->Assign_Target(target);
        infantry->CurrentMission = MISSION_CAPTURE; infantry->MissionQueue = MISSION_NONE;
        infantry->Per_Cell_Process(PCP_END);
        check("allowed forced capture transfers ownership", target_techno->House == new_owner);
        check("allowed forced capture consumes infantry", !present(infantry) || !infantry->IsActive || infantry->IsInLimbo);
        check("allowed forced capture emits entry event", entries > before);
    }
    watched = nullptr;
    std::fprintf(output, "RESULT checks=%d failures=%d entry_events=%d\n", checks, failures, entries);
    std::fflush(output);
    std::fclose(output);
    output = nullptr;
    // This isolated process contains artificial order/position state; do not save it.
    ExitProcess(failures ? 1 : 0);
    return 0;
}
