// Fill out your copyright notice in the Description page of Project Settings.


#include "SlideNavModeComponent.h"

#include "DashNavModeComponent.h"
#include "LedgeVaultNavModeComponent.h"
#include "ModularCharacterMovementComponent.h"
#include "ModularNavigationCharacter.h"
#include "ModularNavigationUtils.h"
#include "Engine/NetSerialization.h"
#include "GameFramework/GameNetworkManager.h"


void FSlideNavModeNetworkSavedData::Clear()
{
	FNavModeNetworkSavedData::Clear();
	ImpulseRemaining = 0.f;
	LastVelocity = FVector::Zero();
	bLastWasAirSlide = false;
}

void FSlideNavModeNetworkSavedData::RecordPostUpdate(ACharacter* Char, UNavModeComponent* NavMode,
	FSavedMove_Character::EPostUpdateMode PostUpdateMode)
{
	FNavModeNetworkSavedData::RecordPostUpdate(Char, NavMode, PostUpdateMode);
	const USlideNavModeComponent* SlideNavMode = Cast<USlideNavModeComponent>(NavMode);
	ImpulseRemaining = SlideNavMode->ImpulseRemaining;
	LastVelocity = SlideNavMode->LastVelocity;
	bLastWasAirSlide = SlideNavMode->bLastWasAirSlide;
}

void FSlideNavModeNetworkSavedData::Serialize(FArchive& Ar, UPackageMap* PackageMap)
{
	FNavModeNetworkSavedData::Serialize(Ar, PackageMap);
	const bool bIsSaving = Ar.IsSaving();
	bool bLocalSuccess = true;
	FModularNavigationUtils::NetSerializeFloat(Ar, ImpulseRemaining, 10, bLocalSuccess);
	LastVelocity.NetSerialize(Ar, PackageMap, bLocalSuccess);
	SerializeOptionalValue<bool>(bIsSaving, Ar, bLastWasAirSlide, false);
}

void FSlideNavModeNetworkSavedData::Fill(ACharacter* Char, UNavModeComponent* NavMode) const
{
	FNavModeNetworkSavedData::Fill(Char, NavMode);
	USlideNavModeComponent* SlideNavMode = Cast<USlideNavModeComponent>(NavMode);
	SlideNavMode->ImpulseRemaining = ImpulseRemaining;
	SlideNavMode->LastVelocity = LastVelocity;
	SlideNavMode->bLastWasAirSlide = bLastWasAirSlide;
}

bool FSlideNavModeNetworkSavedData::CheckForError(ACharacter* Char, UNavModeComponent* NavMode) const
{
	if (FNavModeNetworkSavedData::CheckForError(Char, NavMode))
		return true;

	const USlideNavModeComponent* SlideNavMode = Cast<USlideNavModeComponent>(NavMode);
	// TODO constant
	if (SlideNavMode->ImpulseRemaining - ImpulseRemaining > 1.f/1000.f)
		return true;

	// TODO not location
	const AGameNetworkManager* GameNetworkManager =
		static_cast<const AGameNetworkManager*>(AGameNetworkManager::StaticClass()->GetDefaultObject());
	if (GameNetworkManager->ExceedsAllowablePositionError(SlideNavMode->LastVelocity - LastVelocity))
		return true;
	
	if (SlideNavMode->bLastWasAirSlide != bLastWasAirSlide)
		return true;
	
	return false;
}

FNavModeNetworkSavedData* FSlideNavModeNetworkSavedData::NewInstance() const
{
	return new FSlideNavModeNetworkSavedData();
}

void FSlideNavModeNetworkSavedData::CopyFrom(const FNavModeNetworkSavedData* Original)
{
	FNavModeNetworkSavedData::CopyFrom(Original);
	const FSlideNavModeNetworkSavedData* OriginalSlide = static_cast<const FSlideNavModeNetworkSavedData*>(Original);
	ImpulseRemaining = OriginalSlide->ImpulseRemaining;
	LastVelocity = OriginalSlide->LastVelocity;
	bLastWasAirSlide = OriginalSlide->bLastWasAirSlide;
}

USlideNavModeComponent::USlideNavModeComponent()
{
	// Defaults
	Friction = 400.f;
	MovementAcceleration = 400.f;
	EnterThreshold = 500.f;
	ExitThreshold = 250.f;
	Impulse = 300.f;
	ImpulseRecoveryTime = 1.f;
	MaxImpulseSpeed = 1200.f;
}

float USlideNavModeComponent::GetImpulse() const
{
	return Impulse;
}

void USlideNavModeComponent::SetImpulse(const float InImpulse)
{
	Impulse = InImpulse;
	ImpulseRemaining = Impulse;
}

void USlideNavModeComponent::BeginPlay()
{
	Super::BeginPlay();

	ImpulseRemaining = Impulse;
}

bool USlideNavModeComponent::CanBegin()
{
	TArray<UDashNavModeComponent*> DashNavModes;
	GetCharacter()->GetComponents<UDashNavModeComponent*>(DashNavModes);

	for (const UDashNavModeComponent* DashNavMode : DashNavModes)
	{
		if (DashNavMode->IsModeActive()) return false;
	}
	
	TArray<ULedgeVaultNavModeComponent*> LedgeVaultNavModes;
	GetCharacter()->GetComponents<ULedgeVaultNavModeComponent*>(LedgeVaultNavModes);

	for (const ULedgeVaultNavModeComponent* LedgeVaultNavMode : LedgeVaultNavModes)
	{
		if (LedgeVaultNavMode->IsModeActive()) return false;
	}
	
	return GetCharacterMovement()->MovementMode != MOVE_Walking
		|| (GetCharacterMovement()->MovementMode == MOVE_Walking && GetCharacterMovement()->Velocity.Length() >= EnterThreshold);
}

void USlideNavModeComponent::Begin()
{
	Super::Begin();

	FMovementParameterContext MPC = NeutralContext();

	// need full control of how the separate friction layers work
	MPC.bUseSeparateBrakingFriction = true;
	// the maximum acceleration when inputting movement
	MPC.MaxAcceleration = 0;
	// this is the actual constant friction, will be calculated on the fly in TickComponent (based on slope normal)
	MPC.MaxBrakingDeceleration = 0;
	// no braking while sliding
	MPC.BrakingFriction = 0;
	MPC.BrakingFrictionFactor = 0;
	// friction which scales with velocity, this is not what we want
	MPC.GroundFriction = 0;
	// sliding should preserve full momentum
	MPC.WalkOntoSteepSlopeVelocityScale = 1.f;
	
	GetCharacterMovement()->UseMovementParameterContext(MPC);

	if (ImpulseRemaining > 0.f && GetCharacterMovement()->IsMovingOnGround())
	{
		const FVector CurrentVelocity = FVector(GetCharacterMovement()->Velocity.X, GetCharacterMovement()->Velocity.Y, 0);
		const float ImpulseToApply = FMath::Min(ImpulseRemaining, FMath::Max(0.f, MaxImpulseSpeed - CurrentVelocity.Length()));
		GetCharacterMovement()->AddImpulse(ImpulseToApply * CurrentVelocity.GetSafeNormal(),true);
		
		ImpulseRemaining = 0.f;
	}
}

void USlideNavModeComponent::Exec(float DeltaTime)
{
	if (!GetCharacterMovement()->GetLastInputVector().IsNearlyZero(.1f))
	{
		const float SpeedBeforeInput = GetCharacterMovement()->Velocity.Length();
		GetCharacterMovement()->Velocity +=
			MovementAcceleration * GetCharacterMovement()->GetLastInputVector() * DeltaTime;
		if (GetCharacterMovement()->Velocity.Length() > SpeedBeforeInput)
			GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.GetSafeNormal() * SpeedBeforeInput;
	}
	
	FMovementParameterContext MPC = GetMovementParameterContext();
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		if (FMath::Abs(GetCharacterMovement()->Velocity.Z) < UE_KINDA_SMALL_NUMBER && LastVelocity.Z < -UE_KINDA_SMALL_NUMBER)
		{
			// We just hit the floor from the air, apply impulse towards the incline of the slope (UE stores ground velocity as a flat horizontal vector)
			const FVector ReactiveImpulse = FModularNavigationUtils::Component(LastVelocity.Z * FVector::DownVector, GetCharacterMovement()->CurrentFloor.HitResult.Normal);
			GetCharacterMovement()->AddImpulse(FVector(ReactiveImpulse.X, ReactiveImpulse.Y, 0), true);
		}
		
		const float FrictionOnSlope = Friction * FVector::DotProduct(GetCharacterMovement()->CurrentFloor.HitResult.Normal, FVector::UpVector);
		MPC.MaxBrakingDeceleration = FrictionOnSlope;

		const FVector GravityForce = FMath::Abs(GetCharacterMovement()->Mass * GetCharacterMovement()->GetGravityZ()) * FVector::DownVector;
		const FVector ReactiveForce = FModularNavigationUtils::Component(-GravityForce, GetCharacterMovement()->CurrentFloor.HitResult.Normal);
		GetCharacterMovement()->AddForce(GravityForce + ReactiveForce);
	}
	GetCharacterMovement()->UseMovementParameterContext(MPC);
	
	// ImpulseRemaining += FUtils::Adjustment(ImpulseRemaining, Impulse, DeltaTime, Impulse / ImpulseRecoveryTime);

	LastVelocity = GetCharacterMovement()->Velocity;
	
	Super::Exec(DeltaTime);
}

bool USlideNavModeComponent::ShouldEnd()
{
	return GetCharacterMovement()->Velocity.Length() < ExitThreshold;
} 

void USlideNavModeComponent::End()
{
	GetCharacterMovement()->UnsetMovementParameterContext();
	Super::End();
	
	LastVelocity = FVector::ZeroVector;
}

void USlideNavModeComponent::Idle(float DeltaTime)
{
	//UE_LOG(LogTemp, Warning, TEXT("USlideNavModeComponent::Idle(%f)"), DeltaTime);
	if (ImpulseRemaining < Impulse)
		ImpulseRemaining += FMath::Min(DeltaTime * Impulse / ImpulseRecoveryTime, Impulse - ImpulseRemaining);
}

bool USlideNavModeComponent::AllowsAirLedgeStep()
{
	return false;
}

FString USlideNavModeComponent::GetDebugInfo()
{
	return FString::Format(TEXT("{0}, ImpulseRemaining = {1}, LastVelocity = {2}, bLastWasAirSlide = {3}"),
		{Super::GetDebugInfo(), ImpulseRemaining, LastVelocity.ToString(), bLastWasAirSlide});
}
