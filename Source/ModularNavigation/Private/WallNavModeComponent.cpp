// Fill out your copyright notice in the Description page of Project Settings.


#include "..\Public\WallNavModeComponent.h"

#include "DashNavModeComponent.h"
#include "LedgeVaultNavModeComponent.h"
#include "ModularCharacterMovementComponent.h"
#include "ModularNavigationCharacter.h"
#include "ModularNavigationUtils.h"
#include "Components/CapsuleComponent.h"


class ULedgeVaultNavModeComponent;

// void FWallNavModeNetworkPilotData::Clear()
// {
// 	FNavModeNetworkPilotData::Clear();
// }
//
// void FWallNavModeNetworkPilotData::RecordPreflight(ACharacter* Char, UNavModeComponent* NavMode, float InDeltaTime)
// {
// 	FNavModeNetworkPilotData::RecordPreflight(Char, NavMode, InDeltaTime);
// 	const UWallNavModeComponent* WallNavMode = Cast<UWallNavModeComponent>(NavMode);
// }
//
// void FWallNavModeNetworkPilotData::Serialize(FArchive& Ar, UPackageMap* PackageMap)
// {
// 	FNavModeNetworkPilotData::Serialize(Ar, PackageMap);
// 	const bool bIsSaving = Ar.IsSaving();
// 	bool bLocalSuccess = true;
// }
//
// void FWallNavModeNetworkPilotData::Fill(ACharacter* Char, UNavModeComponent* NavMode) const
// {
// 	FNavModeNetworkPilotData::Fill(Char, NavMode);
// 	UWallNavModeComponent* WallNavMode = Cast<UWallNavModeComponent>(NavMode);
// 	WallNavMode->WallNavigationInfo = WallNavigationInfo;
// }
//
// FNavModeNetworkPilotData* FWallNavModeNetworkPilotData::NewInstance() const
// {
// 	return new FWallNavModeNetworkPilotData();
// }
//
// void FWallNavModeNetworkPilotData::CopyFrom(const FNavModeNetworkPilotData* Original)
// {
// 	FNavModeNetworkPilotData::CopyFrom(Original);
// 	const FWallNavModeNetworkPilotData* OriginalWall = static_cast<const FWallNavModeNetworkPilotData*>(Original);
// }

void FWallNavModeNetworkSavedData::Clear()
{
	FNavModeNetworkSavedData::Clear();
	WallNavInfo = FWallNavInfo();
	TimeSinceJump = 0.f;
	Timer = 0.f;
	LastUsedWallNavInfo = FWallNavInfo();
	TimeSinceNavEnd = 0.f;
}

void FWallNavModeNetworkSavedData::RecordPostUpdate(ACharacter* Char, UNavModeComponent* NavMode,
	FSavedMove_Character::EPostUpdateMode PostUpdateMode)
{
	FNavModeNetworkSavedData::RecordPostUpdate(Char, NavMode, PostUpdateMode);
	const UWallNavModeComponent* WallNavMode = Cast<UWallNavModeComponent>(NavMode);
	WallNavInfo = WallNavMode->WallNavInfo;
	TimeSinceJump = WallNavMode->TimeSinceJump;
	Timer = WallNavMode->Timer;
	LastUsedWallNavInfo = WallNavMode->LastUsedWallNavInfo;
	TimeSinceNavEnd = WallNavMode->TimeSinceNavEnd;
}

void FWallNavModeNetworkSavedData::Serialize(FArchive& Ar, UPackageMap* PackageMap)
{
	FNavModeNetworkSavedData::Serialize(Ar, PackageMap);
	const bool bIsSaving = Ar.IsSaving();
	bool bLocalSuccess = true;

	bool WallNavigationInfo_bValid = WallNavInfo.bValid;
	FVector_NetQuantizeNormal WallNavigationInfo_ImpactNormal = WallNavInfo.ImpactNormal;
	SerializeOptionalValue<bool>(bIsSaving, Ar, WallNavigationInfo_bValid, false);
	WallNavigationInfo_ImpactNormal.NetSerialize(Ar, PackageMap, bLocalSuccess);
	if (!bIsSaving)
	{
		WallNavInfo = WallNavigationInfo_bValid ? FWallNavInfo(WallNavigationInfo_ImpactNormal) : FWallNavInfo();
	}
	
	FModularNavigationUtils::NetSerializeFloat(Ar, TimeSinceJump, 10, bLocalSuccess);
	FModularNavigationUtils::NetSerializeFloat(Ar, Timer, 10, bLocalSuccess);
	
	bool LastUsedWallNavigationInfo_bValid = LastUsedWallNavInfo.bValid;
	FVector_NetQuantizeNormal LastUsedWallNavigationInfo_ImpactNormal = LastUsedWallNavInfo.ImpactNormal;
	SerializeOptionalValue<bool>(bIsSaving, Ar, LastUsedWallNavigationInfo_bValid, false);
	LastUsedWallNavigationInfo_ImpactNormal.NetSerialize(Ar, PackageMap, bLocalSuccess);
	if (!bIsSaving)
	{
		LastUsedWallNavInfo = LastUsedWallNavigationInfo_bValid ? FWallNavInfo(LastUsedWallNavigationInfo_ImpactNormal) : FWallNavInfo();
	}
	
	FModularNavigationUtils::NetSerializeFloat(Ar, TimeSinceNavEnd, 10, bLocalSuccess);
}

void FWallNavModeNetworkSavedData::Fill(ACharacter* Char, UNavModeComponent* NavMode) const
{
	FNavModeNetworkSavedData::Fill(Char, NavMode);
	UWallNavModeComponent* WallNavMode = Cast<UWallNavModeComponent>(NavMode);
	WallNavMode->WallNavInfo = WallNavInfo;
	WallNavMode->TimeSinceJump = TimeSinceJump;
	WallNavMode->Timer = Timer;
	WallNavMode->LastUsedWallNavInfo = LastUsedWallNavInfo;
	WallNavMode->TimeSinceNavEnd = TimeSinceNavEnd;
}

bool FWallNavModeNetworkSavedData::CheckForError(ACharacter* Char, UNavModeComponent* NavMode) const
{
	if (FNavModeNetworkSavedData::CheckForError(Char, NavMode))
		return true;

	const UWallNavModeComponent* WallNavMode = Cast<UWallNavModeComponent>(NavMode);
	
	if (WallNavMode->WallNavInfo.bValid != WallNavInfo.bValid)
		return true;
	// TODO constant
	if (FModularNavigationUtils::AngleInDegrees(WallNavMode->WallNavInfo.ImpactNormal, WallNavInfo.ImpactNormal) > 1.f)
		return true;
	
	// TODO constant
	if (WallNavMode->TimeSinceJump - TimeSinceJump > 1.f/1000.f)
		return true;
	if (WallNavMode->Timer - Timer > 1.f/1000.f)
		return true;
	
	if (WallNavMode->LastUsedWallNavInfo.bValid != LastUsedWallNavInfo.bValid)
		return true;
	// TODO constant
	if (FModularNavigationUtils::AngleInDegrees(WallNavMode->LastUsedWallNavInfo.ImpactNormal, LastUsedWallNavInfo.ImpactNormal) > 1.f)
		return true;
	
	// TODO constant
	if (WallNavMode->TimeSinceNavEnd - TimeSinceNavEnd > 1.f/1000.f)
		return true;
	
	return false;
}

FNavModeNetworkSavedData* FWallNavModeNetworkSavedData::NewInstance() const
{
	return new FWallNavModeNetworkSavedData();
}

void FWallNavModeNetworkSavedData::CopyFrom(const FNavModeNetworkSavedData* Original)
{
	FNavModeNetworkSavedData::CopyFrom(Original);
	const FWallNavModeNetworkSavedData* OriginalWall = static_cast<const FWallNavModeNetworkSavedData*>(Original);
	WallNavInfo = OriginalWall->WallNavInfo;
	TimeSinceJump = OriginalWall->TimeSinceJump;
	Timer = OriginalWall->Timer;
	LastUsedWallNavInfo = OriginalWall->LastUsedWallNavInfo;
	TimeSinceNavEnd = OriginalWall->TimeSinceNavEnd;
}

UWallNavModeComponent::UWallNavModeComponent()
{
	// Defaults
	WallGripAccel = 800.f;
	Duration = 1.6f;
	bResetJumps = true;
	MinimumEntrySpeed = -800.f;
	MinimumIncline = 60.f;
	DisallowedRepeatWallAngleThreshold = 30.f;
	ClimbSpeed = 500.f;
	ClimbAccel = 2500.f;
	bLimitClimbSpeed = false;
	EndClimbSpeed = -150.f;
	MomentumClimbAccelMulti = .1f;
	RunSpeed = 0.f;
	RunAccel = 2500.f;
	bLimitRunSpeed = true;
	SpeedThreshold = 0.f;

	// TODO Defaults for wall running (as opposed to climbing)
	// WallGripAccel = 800.f;
	// Duration = 4.f;
	// bResetJumps = true;
	// MinimumEntrySpeed = -800.f;
	// MinimumIncline = 60.f;
	// DisallowedRepeatWallAngleThreshold = 30.f;
	// ClimbSpeed = 200.f;
	// ClimbAccel = 1300.f;
	// bLimitClimbSpeed = false;
	// EndClimbSpeed = -300.f;
	// MomentumClimbAccelMulti = .1f;
	// RunSpeed = 1000.f;
	// RunAccel = 2500.f;
	// bLimitRunSpeed = false;
	// SpeedThreshold = 350.f;

	// Defaults for wall jumping
	WallJumpLateralVelocity = 700.f;
	WallJumpAirControl = 1.0f;
	WallJumpAirControlRecoveryTime = 0.f;
	bResetWallNavOnWallJump = true;
	WallJumpInputInfluence = 0.f;

	// State
	TimeSinceJump = 0.f;
}

void UWallNavModeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	ResetTimer();
	
	GetCharacter()->GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &UWallNavModeComponent::OnHitCallback);
}

void UWallNavModeComponent::OnHitCallback(UPrimitiveComponent* HitComponent, AActor* OtherActor,
										  UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	HitImpactNormal = Hit.ImpactNormal;
	AcceptWallNavInfoIfValid(FWallNavInfo(HitImpactNormal));
}

void UWallNavModeComponent::Preflight()
{
	Super::Preflight();
	// TODO 
	// AcceptWallNavigationInfoIfValid(FWallNavigationInfo(HitImpactNormal));
}

bool UWallNavModeComponent::CanBegin()
{
	// TODO
	// AcceptWallNavigationInfoIfValid(FWallNavigationInfo(HitImpactNormal));
	if (!WallNavInfo.bValid) return false;
	
	TArray<ULedgeVaultNavModeComponent*> LedgeVaultNavModes;
	GetCharacter()->GetComponents<ULedgeVaultNavModeComponent*>(LedgeVaultNavModes);
	for (ULedgeVaultNavModeComponent* LedgeVaultNavMode : LedgeVaultNavModes)
	{
		if (LedgeVaultNavMode->IsModeActive()) return false;
	}
	
	TArray<UWallNavModeComponent*> WallNavModes;
	GetCharacter()->GetComponents<UWallNavModeComponent*>(WallNavModes);
	for (UWallNavModeComponent* WallNavMode : WallNavModes)
	{
		if (WallNavMode != this && WallNavMode->IsModeActive()) return false;
		if (WallNavMode->TimeSinceJump < WALL_NAV_END_GRACE_PERIOD) return false;
	}

	if (SpeedThreshold > 0.f
		&& FVector::VectorPlaneProject(GetCharacter()->GetVelocity(), WallNavInfo.WallInfluence).Length() < SpeedThreshold)
		return false;
	
	FHitResult Hit(ForceInit);
	const UCapsuleComponent* CapsuleComponent = GetCharacter()->GetCapsuleComponent();
	const FCollisionQueryParams Params = FCollisionQueryParams(FName(TEXT("Wall Run Trace")), true, GetCharacter());
	bool IsWallHit = GetWorld()->LineTraceSingleByChannel(Hit,
		CapsuleComponent->GetComponentLocation(),
		CapsuleComponent->GetComponentLocation()
			- WallNavInfo.ImpactNormal * CapsuleComponent->GetScaledCapsuleRadius() * 2.f,
		ECC_Visibility,
		Params);

	if (IsWallHit)
	{
		IsWallHit = AcceptWallNavInfoIfValid(FWallNavInfo(Hit.ImpactNormal));
	} else
	{
		WallNavInfo = FWallNavInfo();
	}
	
	const bool IsInAir = GetCharacterMovement()->IsFalling();

	bool HasEntrySpeed = false;
	if (IsWallHit)
	{
		const FVector EntryVelocity = FModularNavigationUtils::Component(GetCharacterMovement()->Velocity, WallNavInfo.WallInfluence);
		float EntrySpeed = EntryVelocity.Length();
		if (FModularNavigationUtils::AngleInDegrees(GetCharacterMovement()->Velocity, WallNavInfo.WallInfluence) > 90)
		{
			EntrySpeed *= -1;
		}
		if (EntrySpeed > MinimumEntrySpeed) HasEntrySpeed = true;
	}
	
	return IsWallHit && IsInAir && Timer > 0.f && HasEntrySpeed;
}

void UWallNavModeComponent::Begin()
{
	Super::Begin();

	GetCharacterMovement()->ResetAirControlDampening();
	
	TArray<UWallNavModeComponent*> WallNavModes;
	GetCharacter()->GetComponents<UWallNavModeComponent*>(WallNavModes);
	WallNavModes.Remove(this);
	
	// for (UWallNavModeComponent* WallNavMode : WallNavModes)
	// {
	// 	WallNavMode->Timer = 0;
	// }
	
	if (bResetJumps)
	{
		GetCharacter()->ResetJumpState();
		GetCharacter()->JumpCurrentCount = 0;
		GetCharacter()->JumpCurrentCountPreJump = 0;
	}

	LastUsedWallNavInfo = WallNavInfo;
}

void UWallNavModeComponent::Exec(float DeltaTime)
{
	// Grip wall
	GetCharacterMovement()->AddForce(-WallNavInfo.ImpactNormal * WallGripAccel * GetCharacterMovement()->Mass);

	// Climb acceleration
	float ClimbAccelThisTick = ClimbAccel;
	
	float ClimbSpeedDiff = FMath::Lerp(EndClimbSpeed, ClimbSpeed, Timer/ Duration);
	const float CurrentWallInfSpeed = FModularNavigationUtils::Component(GetCharacterMovement()->Velocity,
		WallNavInfo.WallInfluence).Length();
	
	if (FModularNavigationUtils::AngleInDegrees(GetCharacterMovement()->Velocity, WallNavInfo.WallInfluence) < 90)
	{
		ClimbSpeedDiff -= CurrentWallInfSpeed;
		ClimbAccelThisTick += MomentumClimbAccelMulti * CurrentWallInfSpeed;
	} else
	{
		ClimbSpeedDiff += CurrentWallInfSpeed;
		ClimbAccelThisTick -= MomentumClimbAccelMulti * CurrentWallInfSpeed;
	}
	ClimbAccelThisTick *= DeltaTime;
	
	if (bLimitClimbSpeed && ClimbSpeedDiff < 0)
	{
		ClimbAccelThisTick = -ClimbAccelThisTick;
		ClimbAccelThisTick = FMath::Max(ClimbAccelThisTick, FMath::Min(0.f, ClimbSpeedDiff));
	}
	else ClimbAccelThisTick = FMath::Min(ClimbAccelThisTick, FMath::Max(0.f, ClimbSpeedDiff));
	
	if (ClimbAccelThisTick > 0.f)
	{
		Timer -= DeltaTime;
		AddClimbImpulse(ClimbAccelThisTick, true);
	}

	const FVector CurrentRunVelocity = FModularNavigationUtils::Component(GetCharacterMovement()->Velocity,
		FVector::CrossProduct(WallNavInfo.ImpactNormal, FVector::UpVector));
	const float RunSpeedDiff = RunSpeed - CurrentRunVelocity.Length();
	
	float RunAccelThisTick = RunAccel * DeltaTime;
	
	if (bLimitRunSpeed && RunSpeedDiff < 0)
	{
		RunAccelThisTick = -RunAccelThisTick;
		RunAccelThisTick = FMath::Max(RunAccelThisTick, FMath::Min(0.f, RunSpeedDiff));
	}
	else RunAccelThisTick = FMath::Min(RunAccelThisTick, FMath::Max(0.f, RunSpeedDiff));
	
	GetCharacterMovement()->AddImpulse(CurrentRunVelocity.GetSafeNormal() * RunAccelThisTick, true);

	JumpIdle(DeltaTime);
	
	Super::Exec(DeltaTime);
}

bool UWallNavModeComponent::ShouldEnd()
{
	return !CanBegin();
}

void UWallNavModeComponent::End()
{
	TimeSinceNavEnd = 0.f;
	
	Super::End();
}

void UWallNavModeComponent::Idle(float DeltaTime)
{
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		ResetTimer();
		LastUsedWallNavInfo = FWallNavInfo();
	}
	TimeSinceNavEnd += DeltaTime;

	JumpIdle(DeltaTime);
}

bool UWallNavModeComponent::DoJump(bool bReplayingMoves)
{
	check(WallNavInfo.bValid);

	// UE_LOGFMT(LogModularNavigation, Log, "UWallJumpNavModeComponent::Begin, WallNavigationInfo.bValid = {0},"
	// 							  " WallNavigationInfo.WallNormal = {1}, LocalRole = {2}, RemoteRole = {3}",
	// ActiveWallNavMode->WallNavigationInfo.bValid, ActiveWallNavMode->WallNavigationInfo.WallNormal.ToString(),
	// GetCharacter()->GetLocalRole(), GetCharacter()->GetRemoteRole());

	FVector LateralJumpInput = GetCharacterMovement()->GetLastInputVector();
	if (LateralJumpInput.IsNearlyZero())
	{
		LateralJumpInput = GetCharacter()->GetActorForwardVector();
	}
	if (FModularNavigationUtils::AngleInDegrees(LateralJumpInput, WallNavInfo.ImpactNormal) > 90)
	{
		const FVector ForwardsTravelDirection = FModularNavigationUtils::Component(GetCharacterMovement()->Velocity,
		FVector::CrossProduct(WallNavInfo.ImpactNormal, FVector::UpVector)).GetSafeNormal2D();
		if (FModularNavigationUtils::AngleInDegrees(LateralJumpInput, ForwardsTravelDirection) <= 90)
		{
			LateralJumpInput = ForwardsTravelDirection;
		}
		else
		{
			LateralJumpInput = -ForwardsTravelDirection;
		}
	}
	const FVector LateralJumpDirection = (WallJumpInputInfluence * LateralJumpInput
		+ (1.f - WallJumpInputInfluence) * WallNavInfo.ImpactNormal).GetSafeNormal();
	const FVector LateralJumpVelocity = LateralJumpDirection  * WallJumpLateralVelocity;
	
	GetCharacterMovement()->Velocity = FVector::VectorPlaneProject(GetCharacterMovement()->Velocity,
	WallNavInfo.ImpactNormal);

	if (WallJumpMomentumOverride > 0.f)
	{
		 // TODO better logic for this
		GetCharacterMovement()->Velocity *= 1 - WallJumpMomentumOverride;
	}
	
	GetCharacterMovement()->Velocity.Z = FMath::Max<FVector::FReal>(GetCharacterMovement()->Velocity.Z,
		GetCharacterMovement()->JumpZVelocity);
	GetCharacterMovement()->Velocity += LateralJumpVelocity;

	// TODO better way to do this reset? hit a bug where the server couldn't see that the client jumped because of the reset
	GetCharacter()->ResetJumpState();
	GetCharacter()->bPressedJump = true; // <-- this fixed it though
	GetCharacter()->JumpCurrentCount = 0;
	GetCharacter()->JumpCurrentCountPreJump = 0;

	if (bResetWallNavOnWallJump)
	{
		TArray<UWallNavModeComponent*> WallNavModes;
		GetCharacter()->GetComponents<UWallNavModeComponent*>(WallNavModes);
	
		for (UWallNavModeComponent* WallNavMode : WallNavModes)
		{
			WallNavMode->ResetTimer();
		}
	}

	GetCharacterMovement()->DampenAirControl(WallJumpAirControl, WallJumpAirControlRecoveryTime);

	// Wall nav will end if time since jump was recent
	TimeSinceJump = 0.f;

	return true;
}

bool UWallNavModeComponent::AllowsFirstAirJump()
{
	return bResetJumps;
}

FString UWallNavModeComponent::GetDebugInfo()
{
	return FString::Format(TEXT("{0}, WallNavigationInfo = [{1}], TimeSinceJump = {2}, Timer = {3}, LastUsedWallNavigationInfo = [{4}], TimeSinceNavEnd = {5}"),
		{Super::GetDebugInfo(), WallNavInfo.GetDebugInfo(), TimeSinceJump, Timer, LastUsedWallNavInfo.GetDebugInfo(), TimeSinceNavEnd});
}

bool UWallNavModeComponent::AcceptWallNavInfoIfValid(const FWallNavInfo& Info)
{
	// we've already validated this one
	if (Info == WallNavInfo) return true;
	
	const float WallAngle = FModularNavigationUtils::AngleInDegrees(Info.ImpactNormal, FVector::UpVector);
	const bool IsWithinAngleRange = WallAngle > GetCharacterMovement()->GetWalkableFloorAngle()
		&& WallAngle < 180 - GetCharacterMovement()->GetWalkableFloorAngle() && WallAngle > MinimumIncline;
	const bool IsNotSimilarToLast = TimeSinceNavEnd <= WALL_NAV_END_GRACE_PERIOD
		|| DisallowedRepeatWallAngleThreshold <= 0.f || !LastUsedWallNavInfo.bValid
		|| FModularNavigationUtils::AngleInDegrees(Info.ImpactNormal, LastUsedWallNavInfo.ImpactNormal) >= DisallowedRepeatWallAngleThreshold;
	
	if (IsWithinAngleRange && IsNotSimilarToLast) WallNavInfo = Info;
	return IsWithinAngleRange && IsNotSimilarToLast;
}

void UWallNavModeComponent::AddClimbImpulse(float ImpulseUpwards, bool bVelocityChange) const
{
	check(WallNavInfo.bValid);
	const float ImpulseClimb = ImpulseUpwards
		/ FModularNavigationUtils::Component(WallNavInfo.WallInfluence, FVector::UpVector).Length();
	GetCharacterMovement()->AddImpulse(WallNavInfo.WallInfluence * ImpulseClimb, bVelocityChange);
}

void UWallNavModeComponent::ResetTimer()
{
	Timer = Duration;
}

void UWallNavModeComponent::JumpIdle(float DeltaTime)
{
	TimeSinceJump += DeltaTime;
}

