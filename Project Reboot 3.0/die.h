#pragma once

#include "reboot.h"
#include "FortGameModeAthena.h"
#include "GameplayStatics.h"
#include "CurveTable.h"
#include "KismetStringLibrary.h"s
#include "DataTableFunctionLibrary.h"

static inline void (*SetZoneToIndexOriginal)(AFortGameModeAthena* GameModeAthena, int OverridePhaseMaybeIDFK);

static void SetZoneToIndexHook(AFortGameModeAthena* GameModeAthena, int OverridePhaseMaybeIDFK)
{
	auto GameState = Cast<AFortGameStateAthena>(GameModeAthena->GetGameState());

	if (!GameState)
		return;

	static auto SafeZoneIndicatorOffset = GameModeAthena->GetOffset("SafeZoneIndicator");
	auto SafeZoneIndicator = GameModeAthena->Get<AActor*>(SafeZoneIndicatorOffset);

	static auto SafeZoneFinishShrinkTimeOffset = SafeZoneIndicator->GetOffset("SafeZoneFinishShrinkTime");
	static auto SafeZoneStartShrinkTimeOffset = SafeZoneIndicator->GetOffset("SafeZoneStartShrinkTime");
	static auto RadiusOffset = SafeZoneIndicator->GetOffset("Radius");

	static auto SafeZonePhaseOffset = GameModeAthena->GetOffset("SafeZonePhase");

	static auto MapInfoOffset = GameState->GetOffset("MapInfo");
	auto MapInfo = GameState->Get<AActor*>(MapInfoOffset);

	static auto SafeZoneDefinitionOffset = MapInfo->GetOffset("SafeZoneDefinition");
	auto SafeZoneDefinition = MapInfo->GetPtr<__int64>(SafeZoneDefinitionOffset);

	LOG_INFO(LogDev, "SafeZoneDefinitionOffset: 0x{:x}", SafeZoneDefinitionOffset);

	static auto ZoneDurationsOffset = std::floor(Fortnite_Version) >= 18 ? 0x248 : 0x1F8;
	static auto ZoneHoldDurationsOffset = ZoneDurationsOffset - 0x10;

	auto& ZoneDurations = *(TArray<float>*)(__int64(SafeZoneDefinition) + ZoneDurationsOffset);
	auto& ZoneHoldDurations = *(TArray<float>*)(__int64(SafeZoneDefinition) + ZoneHoldDurationsOffset);

	static bool bFilledDurations = false;

	if (!bFilledDurations)
	{
		bFilledDurations = true;

		auto CurrentPlaylist = GetPlaylistToUse();
		UCurveTable* FortGameData = nullptr;

		static auto GameDataOffset = CurrentPlaylist->GetOffset("GameData");
		FortGameData = CurrentPlaylist ? CurrentPlaylist->Get<TSoftObjectPtr<UCurveTable>>(GameDataOffset).Get() : nullptr;

		if (!FortGameData)
			FortGameData = FindObject<UCurveTable>("/Game/Balance/AthenaGameData.AthenaGameData");

		LOG_INFO(LogDev, "FortGameData: {}", FortGameData ? FortGameData->GetFullName() : "InvalidObject");

		auto ShrinkTimeFName = UKismetStringLibrary::Conv_StringToName(L"Default.SafeZone.ShrinkTime");
		auto HoldTimeFName = UKismetStringLibrary::Conv_StringToName(L"Default.SafeZone.WaitTime");
		FString ContextString;

		/* for (int i = 0; i < 10; i++)
		{
			LOG_INFO(LogDev, "[{}] Value {}", i, FortGameData->GetValueOfKey(FortGameData->GetKey(ShrinkTimeFName, i)));
		} */

		/* for (float i = 0; i < 1.1; i += 0.1)
		{
			float res;
			UDataTableFunctionLibrary::EvaluateCurveTableRow(FortGameData, ShrinkTimeFName, i, ContextString, nullptr, &res);
			LOG_INFO(LogZone, "[{}] {}", i, res);
		} */

		for (int i = 0; i < ZoneDurations.Num(); i++)
		{
			ZoneDurations.at(i) = FortGameData->GetValueOfKey(FortGameData->GetKey(ShrinkTimeFName, i));
		}
		for (int i = 0; i < ZoneHoldDurations.Num(); i++)
		{
			ZoneHoldDurations.at(i) = FortGameData->GetValueOfKey(FortGameData->GetKey(HoldTimeFName, i));
		}

		for (int i = 0; i < ZoneDurations.Num(); i++)
		{
			LOG_INFO(LogZone, "Move [{}] {}", i, ZoneDurations.at(i));
		}

		for (int i = 0; i < ZoneHoldDurations.Num(); i++)
		{
			LOG_INFO(LogZone, "Hold [{}] {}", i, ZoneHoldDurations.at(i));
		}

		/*
		if (ZoneDurations.ArrayNum >= 1) ZoneDurations.at(0) = 0;
		if (ZoneDurations.ArrayNum >= 2) ZoneDurations.at(1) = 180;
		if (ZoneDurations.ArrayNum >= 3) ZoneDurations.at(2) = 120;
		if (ZoneDurations.ArrayNum >= 4) ZoneDurations.at(3) = 90;
		if (ZoneDurations.ArrayNum >= 5) ZoneDurations.at(4) = 70;
		if (ZoneDurations.ArrayNum >= 6) ZoneDurations.at(5) = 60;
		if (ZoneDurations.ArrayNum >= 7) ZoneDurations.at(6) = 60;
		if (ZoneDurations.ArrayNum >= 8) ZoneDurations.at(7) = 55;
		if (ZoneDurations.ArrayNum >= 9) ZoneDurations.at(8) = 45;
		if (ZoneDurations.ArrayNum >= 10) ZoneDurations.at(9) = 75;
		if (ZoneDurations.ArrayNum >= 11) ZoneDurations.at(10) = 0;

		if (ZoneHoldDurations.ArrayNum >= 1) ZoneHoldDurations.at(0) = 0;
		if (ZoneHoldDurations.ArrayNum >= 2) ZoneHoldDurations.at(1) = 170;
		if (ZoneHoldDurations.ArrayNum >= 3) ZoneHoldDurations.at(2) = 120;
		if (ZoneHoldDurations.ArrayNum >= 4) ZoneHoldDurations.at(3) = 90;
		if (ZoneHoldDurations.ArrayNum >= 5) ZoneHoldDurations.at(4) = 80;
		if (ZoneHoldDurations.ArrayNum >= 6) ZoneHoldDurations.at(5) = 50;
		if (ZoneHoldDurations.ArrayNum >= 7) ZoneHoldDurations.at(6) = 30;
		if (ZoneHoldDurations.ArrayNum >= 8) ZoneHoldDurations.at(7) = 0;
		if (ZoneHoldDurations.ArrayNum >= 9) ZoneHoldDurations.at(8) = 0;
		if (ZoneHoldDurations.ArrayNum >= 10) ZoneHoldDurations.at(9) = 0;
		if (ZoneHoldDurations.ArrayNum >= 11) ZoneHoldDurations.at(10) = 0;
		*/
	}

	LOG_INFO(LogZone, "SafeZonePhase: {}", GameModeAthena->Get<int>(SafeZonePhaseOffset));
	LOG_INFO(LogZone, "OverridePhaseMaybeIDFK: {}", OverridePhaseMaybeIDFK);
	LOG_INFO(LogZone, "TimeSeconds: {}", UGameplayStatics::GetTimeSeconds(GetWorld()));
	
	SetZoneToIndexOriginal(GameModeAthena, OverridePhaseMaybeIDFK);

	LOG_INFO(LogZone, "SafeZonePhase After: {}", GameModeAthena->Get<int>(SafeZonePhaseOffset));

	float ZoneHoldDuration = 0;

	if (GameModeAthena->Get<int>(SafeZonePhaseOffset) >= 0 && GameModeAthena->Get<int>(SafeZonePhaseOffset) < ZoneHoldDurations.Num())
		ZoneHoldDuration = ZoneHoldDurations.at(GameModeAthena->Get<int>(SafeZonePhaseOffset));

	SafeZoneIndicator->Get<float>(SafeZoneStartShrinkTimeOffset) = GameState->GetServerWorldTimeSeconds() + ZoneHoldDuration;

	float ZoneDuration = 0;

	if (GameModeAthena->Get<int>(SafeZonePhaseOffset) >= 0 && GameModeAthena->Get<int>(SafeZonePhaseOffset) < ZoneDurations.Num())
		ZoneDuration = ZoneDurations.at(GameModeAthena->Get<int>(SafeZonePhaseOffset));

	LOG_INFO(LogZone, "ZoneDuration: {}", ZoneDuration);
	LOG_INFO(LogZone, "Duration: {}", SafeZoneIndicator->Get<float>(RadiusOffset));

	SafeZoneIndicator->Get<float>(SafeZoneFinishShrinkTimeOffset) = SafeZoneIndicator->Get<float>(SafeZoneStartShrinkTimeOffset) + ZoneDuration;
}


void ProcessEventHook(UObject* Object, UFunction* Function, void* Parameters)
{
	if (!Object || !Function)
		return;

	if (Globals::bLogProcessEvent)
	{
		auto FunctionName = Function->GetName(); // UKismetSystemLibrary::GetPathName(Function).ToString();
		auto FunctionFullName = Function->GetFullName();
		auto ObjectName = Object->GetName();

		if (!strstr(FunctionName.c_str(), ("EvaluateGraphExposedInputs")) &&
			!strstr(FunctionName.c_str(), ("Tick")) &&
			!strstr(FunctionName.c_str(), ("OnSubmixEnvelope")) &&
			!strstr(FunctionName.c_str(), ("OnSubmixSpectralAnalysis")) &&
			!strstr(FunctionName.c_str(), ("OnMouse")) &&
			!strstr(FunctionName.c_str(), ("Pulse")) &&
			!strstr(FunctionName.c_str(), ("BlueprintUpdateAnimation")) &&
			!strstr(FunctionName.c_str(), ("BlueprintPostEvaluateAnimation")) &&
			!strstr(FunctionName.c_str(), ("BlueprintModifyCamera")) &&
			!strstr(FunctionName.c_str(), ("BlueprintModifyPostProcess")) &&
			!strstr(FunctionName.c_str(), ("Loop Animation Curve")) &&
			!strstr(FunctionName.c_str(), ("UpdateTime")) &&
			!strstr(FunctionName.c_str(), ("GetMutatorByClass")) &&
			!strstr(FunctionName.c_str(), ("UpdatePreviousPositionAndVelocity")) &&
			!strstr(FunctionName.c_str(), ("IsCachedIsProjectileWeapon")) &&
			!strstr(FunctionName.c_str(), ("LockOn")) &&
			!strstr(FunctionName.c_str(), ("GetAbilityTargetingLevel")) &&
			!strstr(FunctionName.c_str(), ("ReadyToEndMatch")) &&
			!strstr(FunctionName.c_str(), ("ReceiveDrawHUD")) &&
			!strstr(FunctionName.c_str(), ("OnUpdateDirectionalLightForTimeOfDay")) &&
			!strstr(FunctionName.c_str(), ("GetSubtitleVisibility")) &&
			!strstr(FunctionName.c_str(), ("GetValue")) &&
			!strstr(FunctionName.c_str(), ("InputAxisKeyEvent")) &&
			!strstr(FunctionName.c_str(), ("ServerTouchActiveTime")) &&
			!strstr(FunctionName.c_str(), ("SM_IceCube_Blueprint_C")) &&
			!strstr(FunctionName.c_str(), ("OnHovered")) &&
			!strstr(FunctionName.c_str(), ("OnCurrentTextStyleChanged")) &&
			!strstr(FunctionName.c_str(), ("OnButtonHovered")) &&
			!strstr(FunctionName.c_str(), ("ExecuteUbergraph_ThreatPostProcessManagerAndParticleBlueprint")) &&
			!strstr(FunctionName.c_str(), "PinkOatmeal") &&
			!strstr(FunctionName.c_str(), "CheckForDancingAtFish") &&
			!strstr(FunctionName.c_str(), ("UpdateCamera")) &&
			!strstr(FunctionName.c_str(), ("GetMutatorContext")) &&
			!strstr(FunctionName.c_str(), ("CanJumpInternal")) &&
			!strstr(FunctionName.c_str(), ("OnDayPhaseChanged")) &&
			!strstr(FunctionName.c_str(), ("Chime")) &&
			!strstr(FunctionName.c_str(), ("ServerMove")) &&
			!strstr(FunctionName.c_str(), ("OnVisibilitySetEvent")) &&
			!strstr(FunctionName.c_str(), "ReceiveHit") &&
			!strstr(FunctionName.c_str(), "ReadyToStartMatch") &&
			!strstr(FunctionName.c_str(), "K2_GetComponentToWorld") &&
			!strstr(FunctionName.c_str(), "ClientAckGoodMove") &&
			!strstr(FunctionName.c_str(), "Prop_WildWest_WoodenWindmill_01") &&
			!strstr(FunctionName.c_str(), "ContrailCheck") &&
			!strstr(FunctionName.c_str(), "B_StockBattleBus_C") &&
			!strstr(FunctionName.c_str(), "Subtitles.Subtitles_C.") &&
			!strstr(FunctionName.c_str(), "/PinkOatmeal/PinkOatmeal_") &&
			!strstr(FunctionName.c_str(), "BP_SpectatorPawn_C") &&
			!strstr(FunctionName.c_str(), "FastSharedReplication") &&
			!strstr(FunctionName.c_str(), "OnCollisionHitEffects") &&
			!strstr(FunctionName.c_str(), "BndEvt__SkeletalMesh") &&
			!strstr(FunctionName.c_str(), ".FortAnimInstance.AnimNotify_") &&
			!strstr(FunctionName.c_str(), "OnBounceAnimationUpdate") &&
			!strstr(FunctionName.c_str(), "ShouldShowSoundIndicator") &&
			!strstr(FunctionName.c_str(), "Primitive_Structure_AmbAudioComponent_C") &&
			!strstr(FunctionName.c_str(), "PlayStoppedIdleRotationAudio") &&
			!strstr(FunctionName.c_str(), "UpdateOverheatCosmetics") &&
			!strstr(FunctionName.c_str(), "StormFadeTimeline__UpdateFunc") &&
			!strstr(FunctionName.c_str(), "BindVolumeEvents") &&
			!strstr(FunctionName.c_str(), "UpdateStateEvent") &&
			!strstr(FunctionName.c_str(), "VISUALS__UpdateFunc") &&
			!strstr(FunctionName.c_str(), "Flash__UpdateFunc") &&
			!strstr(FunctionName.c_str(), "SetCollisionEnabled") &&
			!strstr(FunctionName.c_str(), "SetIntensity") &&
			!strstr(FunctionName.c_str(), "Storm__UpdateFunc") &&
			!strstr(FunctionName.c_str(), "CloudsTimeline__UpdateFunc") &&
			!strstr(FunctionName.c_str(), "SetRenderCustomDepth") &&
			!strstr(FunctionName.c_str(), "K2_UpdateCustomMovement") &&
			!strstr(FunctionName.c_str(), "AthenaHitPointBar_C.Update") &&
			!strstr(FunctionName.c_str(), "ExecuteUbergraph_Farm_WeatherVane_01") &&
			!strstr(FunctionName.c_str(), "HandleOnHUDElementVisibilityChanged") &&
			!strstr(FunctionName.c_str(), "ExecuteUbergraph_Fog_Machine") &&
			!strstr(FunctionName.c_str(), "ReceiveBeginPlay") &&
			!strstr(FunctionName.c_str(), "OnMatchStarted") &&
			!strstr(FunctionName.c_str(), "CustomStateChanged") &&
			!strstr(FunctionName.c_str(), "OnBuildingActorInitialized") &&
			!strstr(FunctionName.c_str(), "OnWorldReady") &&
			!strstr(FunctionName.c_str(), "OnAttachToBuilding") &&
			!strstr(FunctionName.c_str(), "Clown Spinner") &&
			!strstr(FunctionName.c_str(), "K2_GetActorLocation") &&
			!strstr(FunctionName.c_str(), "GetViewTarget") &&
			!strstr(FunctionName.c_str(), "GetAllActorsOfClass") &&
			!strstr(FunctionName.c_str(), "OnUpdateMusic") &&
			!strstr(FunctionName.c_str(), "Check Closest Point") &&
			!strstr(FunctionName.c_str(), "OnSubtitleChanged__DelegateSignature") &&
			!strstr(FunctionName.c_str(), "OnServerBounceCallback") &&
			!strstr(FunctionName.c_str(), "BlueprintGetInteractionTime") &&
			!strstr(FunctionName.c_str(), "OnServerStopCallback") &&
			!strstr(FunctionName.c_str(), "Light Flash Timeline__UpdateFunc") &&
			!strstr(FunctionName.c_str(), "MainFlightPath__UpdateFunc") &&
			!strstr(FunctionName.c_str(), "PlayStartedIdleRotationAudio") &&
			!strstr(FunctionName.c_str(), "BGA_Athena_FlopperSpawn_") &&
			!strstr(FunctionName.c_str(), "CheckShouldDisplayUI") &&
			!strstr(FunctionName.c_str(), "Timeline_0__UpdateFunc") &&
			!strstr(FunctionName.c_str(), "ClientMoveResponsePacked") &&
			!strstr(FunctionName.c_str(), "ExecuteUbergraph_B_Athena_FlopperSpawnWorld_Placement") &&
			!strstr(FunctionName.c_str(), "Countdown__UpdateFunc") &&
			!strstr(FunctionName.c_str(), "OnParachuteTrailUpdated") &&
			!strstr(FunctionName.c_str(), "Moto FadeOut__UpdateFunc") &&
			!strstr(FunctionName.c_str(), "ExecuteUbergraph_Apollo_GasPump_Valet") &&
			!strstr(FunctionName.c_str(), "GetOverrideMeshMaterial") &&
			!strstr(FunctionName.c_str(), "VendWobble__UpdateFunc") &&
			!strstr(FunctionName.c_str(), "WaitForPawn") &&
			!strstr(FunctionName.c_str(), "FragmentMovement__UpdateFunc") &&
			!strstr(FunctionName.c_str(), "TrySetup") &&
			!strstr(FunctionName.c_str(), "Fade Doused Smoke__UpdateFunc") &&
			!strstr(FunctionName.c_str(), "SetPlayerToSkydive") &&
			!strstr(FunctionName.c_str(), "BounceCar__UpdateFunc") &&
			!strstr(FunctionName.c_str(), "BP_CalendarDynamicPOISelect") &&
			!strstr(FunctionName.c_str(), "OnComponentHit_Event_0") &&
			!strstr(FunctionName.c_str(), "HandleSimulatingComponentHit") &&
			!strstr(FunctionName.c_str(), "CBGA_GreenGlop_WithGrav_C") &&
			!strstr(FunctionName.c_str(), "WarmupCountdownEndTimeUpdated") &&
			!strstr(FunctionName.c_str(), "BP_CanInteract") &&
			!strstr(FunctionName.c_str(), "AthenaHitPointBar_C") &&
			!strstr(FunctionName.c_str(), "ServerFireAIDirectorEvent") &&
			!strstr(FunctionName.c_str(), "BlueprintThreadSafeUpdateAnimation") &&
			!strstr(FunctionName.c_str(), "On Amb Zap Spawn") &&
			!strstr(FunctionName.c_str(), "ServerSetPlayerCanDBNORevive") &&
			!strstr(FunctionName.c_str(), "BGA_Petrol_Pickup_C") &&
			!strstr(FunctionName.c_str(), "GetMutatorsForContextActor") &&
			!strstr(FunctionName.c_str(), "GetControlRotation") &&
			!strstr(FunctionName.c_str(), "K2_GetComponentLocation") &&
			!strstr(FunctionName.c_str(), "MoveFromOffset__UpdateFunc") &&
			!strstr(FunctionFullName.c_str(), "PinkOatmeal_GreenGlop_C") &&
			!strstr(ObjectName.c_str(), "CBGA_GreenGlop_WithGrav_C") &&
			!strstr(FunctionFullName.c_str(), "GCNL_EnvCampFire_Fire_C"))
		{
			LOG_INFO(LogDev, "Function called: {} with {}", FunctionFullName, ObjectName);
		}
	}

	return Object->ProcessEvent(Function, Parameters);
}