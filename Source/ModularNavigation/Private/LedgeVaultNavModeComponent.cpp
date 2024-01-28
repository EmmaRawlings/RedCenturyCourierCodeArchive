// Fill out your copyright notice in the Description page of Project Settings.


#include "LedgeVaultNavModeComponent.h"

#include "DashNavModeComponent.h"
#include "ModularCharacterMovementComponent.h"
#include "ModularNavigationCharacter.h"
#include "ModularNavigationUtils.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/GameNetworkManager.h"
#include "Kismet/KismetSystemLibrary.h"


void FLedgeVaultNavModeNetworkSavedData::Clear()
{
	FNavModeNetworkSavedData::Clear();
	Progress = 0.f;
	From = FVector::Zero();
	To = FVector::Zero();
}

void FLedgeVaultNavModeNetworkSavedData::RecordPostUpdate(ACharacter* Char, UNavModeComponent* NavMode,
	FSavedMove_Character::EPostUpdateMode PostUpdateMode)
{
	FNavModeNetworkSavedData::RecordPostUpdate(Char, NavMode, PostUpdateMode);
	const ULedgeVaultNavModeComponent* LedgeVaultNavMode = Cast<ULedgeVaultNavModeComponent>(NavMode);
	Progress = LedgeVaultNavMode->Progress;
	From = LedgeVaultNavMode->From;
	To = LedgeVaultNavMode->To;
}

void FLedgeVaultNavModeNetworkSavedData::Serialize(FArchive& Ar, UPackageMap* PackageMap)
{
	FNavModeNetworkSavedData::Serialize(Ar, PackageMap);
	bool bLocalSuccess = true;
	FModularNavigationUtils::NetSerializeFloat(Ar, Progress, 10, bLocalSuccess);
	From.NetSerialize(Ar, PackageMap, bLocalSuccess);
	To.NetSerialize(Ar, PackageMap, bLocalSuccess);
}

void FLedgeVaultNavModeNetworkSavedData::Fill(ACharacter* Char, UNavModeComponent* NavMode) const
{
	FNavModeNetworkSavedData::Fill(Char, NavMode);
	ULedgeVaultNavModeComponent* LedgeVaultNavMode = Cast<ULedgeVaultNavModeComponent>(NavMode);
	LedgeVaultNavMode->Progress = Progress;
	LedgeVaultNavMode->From = From;
	LedgeVaultNavMode->To = To;
}

bool FLedgeVaultNavModeNetworkSavedData::CheckForError(ACharacter* Char, UNavModeComponent* NavMode) const
{
	if (FNavModeNetworkSavedData::CheckForError(Char, NavMode))
		return true;

	const ULedgeVaultNavModeComponent* LedgeVaultNavMode = Cast<ULedgeVaultNavModeComponent>(NavMode);
	// TODO constant
	if (LedgeVaultNavMode->Progress - Progress > 1.f/1000.f)
		return true;

	const AGameNetworkManager* GameNetworkManager =
		static_cast<const AGameNetworkManager*>(AGameNetworkManager::StaticClass()->GetDefaultObject());
	if (GameNetworkManager->ExceedsAllowablePositionError(LedgeVaultNavMode->From - From))
		return true;
	if (GameNetworkManager->ExceedsAllowablePositionError(LedgeVaultNavMode->To - To))
		return true;
	
	return false;
}

FNavModeNetworkSavedData* FLedgeVaultNavModeNetworkSavedData::NewInstance() const
{
	return new FLedgeVaultNavModeNetworkSavedData();
}

void FLedgeVaultNavModeNetworkSavedData::CopyFrom(const FNavModeNetworkSavedData* Original)
{
	FNavModeNetworkSavedData::CopyFrom(Original);
	const FLedgeVaultNavModeNetworkSavedData* OriginalLedgeVault = static_cast<const FLedgeVaultNavModeNetworkSavedData*>(Original);
	Progress = OriginalLedgeVault->Progress;
	From = OriginalLedgeVault->From;
	To = OriginalLedgeVault->To;
}

ULedgeVaultNavModeComponent::ULedgeVaultNavModeComponent()
{
	// Defaults
	MaxHeight = 200.f;
	MinHeight = 90.f;
	ForwardsReach = 50.f;
	MaxSlopeAngle = 30.f;
	Duration = .3f;
}

bool ULedgeVaultNavModeComponent::CanBegin()
{
	FVector OutTo;
	const bool CanStart = DetectLedgeToVault(OutTo);
	if (CanStart)
	{
		From = GetCharacter()->GetActorLocation();
		To = OutTo;
	}

	return CanStart;
}

void ULedgeVaultNavModeComponent::Begin()
{
	Super::Begin();

	GetCharacterMovement()->ResetAirControlDampening();
	Progress = 0.f;
}

void ULedgeVaultNavModeComponent::Exec(float DeltaTime)
{
	Progress += DeltaTime / Duration;
	GetCharacter()->SetActorLocation(FMath::Lerp(From, To, FMath::Min(Progress, 1.f)));
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	
	Super::Exec(DeltaTime);
}

bool ULedgeVaultNavModeComponent::ShouldEnd()
{
	return Progress >= 1.f;
} 

void ULedgeVaultNavModeComponent::End()
{
	Progress = -1.f;
	
	Super::End();
}

void ULedgeVaultNavModeComponent::Idle(float DeltaTime)
{
	// do nothing
}

bool ULedgeVaultNavModeComponent::AllowsAirLedgeStep()
{
	return false;
}

FString ULedgeVaultNavModeComponent::GetDebugInfo()
{
	return FString::Format(TEXT("{0}, Progress = {1}, From = {2}, To = {3}"),
		{Super::GetDebugInfo(), Progress, From.ToString(), To.ToString()});
}

bool ULedgeVaultNavModeComponent::DetectLedgeToVault(FVector& OutTo) const
{
	const UCapsuleComponent* CapsuleComponent = GetCharacter()->GetCapsuleComponent();
	TArray<AActor*> ActorsToIgnore = TArray<AActor*>();
	ActorsToIgnore.Add(GetCharacter());
	const FVector CapPos = CapsuleComponent->GetComponentLocation();
	const float CapRadius = CapsuleComponent->GetScaledCapsuleRadius();
	const float CapHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	
	// UE_LOGFMT(LogModularNavigation, Log, "ULedgeVaultNavModeComponent::DetectLedgeToVault, GetActorForwardVector = {0},"
	// 								  " CapPos = {1}, LocalRole = {2}, RemoteRole = {3}",
	// 	GetCharacter()->GetActorForwardVector().ToString(), CapPos.ToString(),
	// 	GetCharacter()->GetLocalRole(), GetCharacter()->GetRemoteRole());

	// scan for ceiling height
	TArray<FHitResult> OutHits;
	const bool IsCeilingHit = UKismetSystemLibrary::CapsuleTraceMulti(
		GetWorld(), CapPos, CapPos + FVector::UpVector * MaxHeight, CapRadius, CapHalfHeight,
		ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None,
		OutHits, true);
	const float TopHeight = IsCeilingHit ? OutHits[0].Location.Z - CapPos.Z : MaxHeight;
	if (TopHeight <= MinHeight) return false;

	for (float ScanDist = LEDGE_VAULT_SEARCH_INCREMENTS; ScanDist < ForwardsReach; ScanDist += LEDGE_VAULT_SEARCH_INCREMENTS)
	{
		// scan for valid ledge
		const FVector ForwardsOffset = GetCharacter()->GetActorForwardVector() * ScanDist;
		const FVector Start = ForwardsOffset + CapPos + FVector::UpVector * TopHeight;
		const FVector End = ForwardsOffset + CapPos + FVector::UpVector * MinHeight;
		OutHits.Empty();
		const bool IsLedgeHit = UKismetSystemLibrary::CapsuleTraceMulti(
			GetWorld(), Start, End, CapRadius, CapHalfHeight, ETraceTypeQuery::TraceTypeQuery1, false,
			ActorsToIgnore, EDrawDebugTrace::None, OutHits, true);
		if (IsLedgeHit
			&& OutHits[0].IsValidBlockingHit()
			&& FModularNavigationUtils::AngleInDegrees(OutHits[0].Normal, FVector::UpVector) <= MaxSlopeAngle)
		{
			// scan for wall
			const FVector ResultPos = OutHits[0].Location;
			OutHits.Empty();
			const bool IsWallHit = UKismetSystemLibrary::CapsuleTraceMulti(
				GetWorld(), ResultPos, FVector(CapPos.X, CapPos.Y, ResultPos.Z), CapRadius, CapHalfHeight,
				ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None,
				OutHits, true);
			if (IsWallHit) return false;
			// valid ledge with no wall in the way
			OutTo = ResultPos;
			return true;
		}
	}
	// TODO shameless copy paste!..
	// scan for valid ledge
	const FVector ForwardsOffset = GetCharacter()->GetActorForwardVector() * ForwardsReach;
	const FVector Start = ForwardsOffset + CapPos + FVector::UpVector * TopHeight;
	const FVector End = ForwardsOffset + CapPos + FVector::UpVector * MinHeight;
	OutHits.Empty();
	const bool IsLedgeHit = UKismetSystemLibrary::CapsuleTraceMulti(
		GetWorld(), Start, End, CapRadius, CapHalfHeight, ETraceTypeQuery::TraceTypeQuery1, false,
		ActorsToIgnore, EDrawDebugTrace::None, OutHits, true);
	if (IsLedgeHit
		&& OutHits[0].IsValidBlockingHit()
		&& FModularNavigationUtils::AngleInDegrees(OutHits[0].Normal, FVector::UpVector) <= MaxSlopeAngle)
	{
		// scan for wall
		const FVector ResultPos = OutHits[0].Location;
		OutHits.Empty();
		const bool IsWallHit = UKismetSystemLibrary::CapsuleTraceMulti(
			GetWorld(), ResultPos, FVector(CapPos.X, CapPos.Y, ResultPos.Z), CapRadius, CapHalfHeight,
			ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None,
			OutHits, true);
		if (IsWallHit) return false;
		// valid ledge with no wall in the way
		OutTo = ResultPos;
		return true;
	}
	//

	// didn't find any ledge to vault
	return false;
}

