// Fill out your copyright notice in the Description page of Project Settings.


#include "NavModeComponent.h"

#include "ModularCharacterMovementComponent.h"
#include "ModularNavigationCharacter.h"
#include "ModularNavigationUtils.h"
#include "Logging/StructuredLog.h"


void FNavModeNetworkPilotData::Clear()
{
	bIsAllowed = false;
}

FNavModeNetworkPilotData* FNavModeNetworkPilotData::Clone() const
{
	FNavModeNetworkPilotData* CloneResult = NewInstance();
	CloneResult->CopyFrom(this);
	return CloneResult;
}

void FNavModeNetworkPilotData::RecordPreflight(ACharacter* Char, UNavModeComponent* NavMode, float InDeltaTime)
{
	bIsAllowed = NavMode->IsAllowed();
}

void FNavModeNetworkPilotData::Serialize(FArchive& Ar, UPackageMap* PackageMap)
{
	const bool bIsSaving = Ar.IsSaving();
	SerializeOptionalValue<bool>(bIsSaving, Ar, bIsAllowed, false);
}

void FNavModeNetworkPilotData::Fill(ACharacter* Char, UNavModeComponent* NavMode) const
{
	NavMode->bIsAllowed = bIsAllowed;
}

FNavModeNetworkPilotData* FNavModeNetworkPilotData::NewInstance() const
{
	return new FNavModeNetworkPilotData();
}

void FNavModeNetworkPilotData::CopyFrom(const FNavModeNetworkPilotData* Original)
{
	bIsAllowed = Original->bIsAllowed;
}

void FNavModeNetworkSavedData::Clear()
{
	// Nothing to do
}

FNavModeNetworkSavedData* FNavModeNetworkSavedData::Clone() const
{
	FNavModeNetworkSavedData* CloneResult = NewInstance();
	CloneResult->CopyFrom(this);
	return CloneResult;
}

void FNavModeNetworkSavedData::RecordPostUpdate(ACharacter* Char, UNavModeComponent* NavMode,
                                                FSavedMove_Character::EPostUpdateMode PostUpdateMode)
{
	// Nothing to do
}

void FNavModeNetworkSavedData::Serialize(FArchive& Ar, UPackageMap* PackageMap)
{
	// Nothing to do
}

void FNavModeNetworkSavedData::Fill(ACharacter* Char, UNavModeComponent* NavMode) const
{
	// nothing to do
}

bool FNavModeNetworkSavedData::CheckForError(ACharacter* Char, UNavModeComponent* NavMode) const
{
	return false;
}

FNavModeNetworkSavedData* FNavModeNetworkSavedData::NewInstance() const
{
	return new FNavModeNetworkSavedData();
}

void FNavModeNetworkSavedData::CopyFrom(const FNavModeNetworkSavedData* Original)
{
	// Nothing to do
}

UNavModeComponent::UNavModeComponent()
{
	TimeActive = 0.f;
	bIsAllowed = false;
}

bool UNavModeComponent::IsModeActive() const
{
	return GetCharacterMovement() != nullptr && GetCharacterMovement()->IsNavModeActive(this);
}

ENavModeActive UNavModeComponent::IfModeActive()
{
	return IsModeActive() ? ENavModeActive::Active : ENavModeActive::Inactive;
}

void UNavModeComponent::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UModularCharacterMovementComponent>();
	checkf(MovementComponent != nullptr, TEXT("UModularCharacterMovementComponent could not be found on %s"), *GetOwner()->GetName());

	CharacterActor = Cast<AModularNavigationCharacter>(GetOwner());
	checkf(CharacterActor != nullptr, TEXT("%s was not of expected type, UModularNavigationCharacter"), *GetOwner()->GetName());
}

void UNavModeComponent::Preflight()
{
	bIsAllowed = IsAllowedNavMode.IsBound() && IsAllowedNavMode.Execute();
}

bool UNavModeComponent::IsAllowed() const
{
	return bIsAllowed;
}

bool UNavModeComponent::CanBegin()
{
	unimplemented();
	return false;
}

void UNavModeComponent::Begin()
{
	UE_LOGFMT(LogModularNavigation, Log, "{0}::Begin physics ({1}), LocalRole = {2}, RemoteRole = {3}",
		*GetFName().ToString(), *GetCharacter()->GetFName().ToString(),
		GetCharacter()->GetLocalRole(), GetCharacter()->GetRemoteRole());
	OnBegin.Broadcast();
	TimeActive = 0.f;
}

void UNavModeComponent::Exec(float DeltaTime)
{
	// nothing to do
	TimeActive += DeltaTime;
}

bool UNavModeComponent::ShouldEnd()
{
	return false;
}

void UNavModeComponent::End()
{
	const float TimeActiveInMs = TimeActive * 1000.f;
	UE_LOGFMT(LogModularNavigation, Log, "{0}::End physics ({1}), LocalRole = {2}, RemoteRole = {3} [{4}ms]",
		*GetFName().ToString(), *GetCharacter()->GetFName().ToString(),
		GetCharacter()->GetLocalRole(), GetCharacter()->GetRemoteRole(), TimeActiveInMs);
	OnEnd.Broadcast();
}

void UNavModeComponent::Idle(float DeltaTime)
{
	// nothing to do
}

bool UNavModeComponent::DoJump(bool bReplayingMoves)
{
	return false;
}

bool UNavModeComponent::AllowsFirstAirJump()
{
	return false;
}

bool UNavModeComponent::AllowsAirLedgeStep()
{
	return true;
}

FString UNavModeComponent::GetDebugInfo()
{
	return FString::Format(TEXT("TimeActive = {0}"), {TimeActive});
}

TObjectPtr<UModularCharacterMovementComponent> UNavModeComponent::GetCharacterMovement() const
{
	return MovementComponent;
}

TObjectPtr<AModularNavigationCharacter> UNavModeComponent::GetCharacter() const
{
	return CharacterActor;
}

FMovementParameterContext UNavModeComponent::NeutralContext() const
{
	return GetCharacterMovement()->NeutralContext();
}

FMovementParameterContext UNavModeComponent::GetMovementParameterContext() const
{
	return GetCharacterMovement()->GetMovementParameterContext();
}
