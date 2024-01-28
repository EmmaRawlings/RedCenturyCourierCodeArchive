// Fill out your copyright notice in the Description page of Project Settings.


#include "DashNavModeComponent.h"

#include "LedgeVaultNavModeComponent.h"
#include "ModularCharacterMovementComponent.h"
#include "ModularNavigationCharacter.h"
#include "ModularNavigationUtils.h"
#include "Logging/StructuredLog.h"

void FDashNavModeNetworkPilotData::Clear()
{
	FNavModeNetworkPilotData::Clear();
	InputDirection = FVector::Zero();
}

void FDashNavModeNetworkPilotData::RecordPreflight(ACharacter* Char, UNavModeComponent* NavMode, float InDeltaTime)
{
	FNavModeNetworkPilotData::RecordPreflight(Char, NavMode, InDeltaTime);
	const UDashNavModeComponent* DashNavMode = Cast<UDashNavModeComponent>(NavMode);
	InputDirection = DashNavMode->InputDirection;
}

void FDashNavModeNetworkPilotData::Serialize(FArchive& Ar, UPackageMap* PackageMap)
{
	FNavModeNetworkPilotData::Serialize(Ar, PackageMap);
	bool bLocalSuccess = true;
	InputDirection.NetSerialize(Ar, PackageMap, bLocalSuccess);
}

void FDashNavModeNetworkPilotData::Fill(ACharacter* Char, UNavModeComponent* NavMode) const
{
	FNavModeNetworkPilotData::Fill(Char, NavMode);
	UDashNavModeComponent* DashNavMode = Cast<UDashNavModeComponent>(NavMode);
	DashNavMode->InputDirection = InputDirection;
}

FNavModeNetworkPilotData* FDashNavModeNetworkPilotData::NewInstance() const
{
	return new FDashNavModeNetworkPilotData();
}

void FDashNavModeNetworkPilotData::CopyFrom(const FNavModeNetworkPilotData* Original)
{
	FNavModeNetworkPilotData::CopyFrom(Original);
	const FDashNavModeNetworkPilotData* OriginalDash = static_cast<const FDashNavModeNetworkPilotData*>(Original);
	InputDirection = OriginalDash->InputDirection;
}

void FDashNavModeNetworkSavedData::Clear()
{
	FNavModeNetworkSavedData::Clear();
	Timer = 0.f;
	CooldownTimer = 0.f;
	Direction = FVector::Zero();
}

void FDashNavModeNetworkSavedData::RecordPostUpdate(ACharacter* Char, UNavModeComponent* NavMode,
	FSavedMove_Character::EPostUpdateMode PostUpdateMode)
{
	FNavModeNetworkSavedData::RecordPostUpdate(Char, NavMode, PostUpdateMode);
	const UDashNavModeComponent* DashNavMode = Cast<UDashNavModeComponent>(NavMode);
	Timer = DashNavMode->Timer;
	CooldownTimer = DashNavMode->CooldownTimer;
	Direction = DashNavMode->Direction;
}

void FDashNavModeNetworkSavedData::Serialize(FArchive& Ar, UPackageMap* PackageMap)
{
	FNavModeNetworkSavedData::Serialize(Ar, PackageMap);
	bool bLocalSuccess = true;
	FModularNavigationUtils::NetSerializeFloat(Ar, Timer, 10, bLocalSuccess);
	FModularNavigationUtils::NetSerializeFloat(Ar, CooldownTimer, 10, bLocalSuccess);
	Direction.NetSerialize(Ar, PackageMap, bLocalSuccess);
}

void FDashNavModeNetworkSavedData::Fill(ACharacter* Char, UNavModeComponent* NavMode) const
{
	FNavModeNetworkSavedData::Fill(Char, NavMode);
	UDashNavModeComponent* DashNavMode = Cast<UDashNavModeComponent>(NavMode);
	DashNavMode->Timer = Timer;
	DashNavMode->CooldownTimer = CooldownTimer;
	DashNavMode->Direction = Direction;
}

bool FDashNavModeNetworkSavedData::CheckForError(ACharacter* Char, UNavModeComponent* NavMode) const
{
	if (FNavModeNetworkSavedData::CheckForError(Char, NavMode))
		return true;

	const UDashNavModeComponent* DashNavMode = Cast<UDashNavModeComponent>(NavMode);
	// TODO constant
	if (DashNavMode->Timer - Timer > 1.f/1000.f)
		return true;
	if (DashNavMode->CooldownTimer - CooldownTimer > 1.f/1000.f)
		return true;
	// TODO constant
	if (FModularNavigationUtils::AngleInDegrees(DashNavMode->Direction, Direction) > 1.f)
		return true;
	
	return false;
}

FNavModeNetworkSavedData* FDashNavModeNetworkSavedData::NewInstance() const
{
	return new FDashNavModeNetworkSavedData();
}

void FDashNavModeNetworkSavedData::CopyFrom(const FNavModeNetworkSavedData* Original)
{
	FNavModeNetworkSavedData::CopyFrom(Original);
	const FDashNavModeNetworkSavedData* OriginalDash = static_cast<const FDashNavModeNetworkSavedData*>(Original);
	Timer = OriginalDash->Timer;
	CooldownTimer = OriginalDash->CooldownTimer;
	Direction = OriginalDash->Direction;
}

UDashNavModeComponent::UDashNavModeComponent()
{
	// Defaults
	Distance = 200.f;
	Duration = .1f;
	AirDistanceRatio = 1.f;
	AirDurationRatio = 1.f;
	bLateralOnly = true;
	MaxInheritFactor = .3f;
	MinimumAirControlForAirDash = .5f;
	bAirControlAffectsAirDashDistance = true;
	AirDashController = CreateDefaultSubobject<UAirDashTiedToAirJumpsNavController>(TEXT("AirDashController"));
	Cooldown = .15f;
	bUseTravelCurveOnGround = false;
	bUseTravelCurveInAir = false;
	TravelCurve = CreateDefaultSubobject<UCurveFloat>(TEXT("TravelCurve"));

	// State
	Timer = -1.f;
	CooldownTimer = 0.f;
	LastTravelTimelineValue = 0.f;
	LastTravelTimelineTime = 0.f;
	bInAirDash = false;
}

void UDashNavModeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (TravelCurve)
	{
		FOnTimelineFloat TravelProgressFunction;
		TravelProgressFunction.BindUFunction(this, FName("HandleTravelProgress"));
		TravelTimeline.AddInterpFloat(TravelCurve, TravelProgressFunction);
		TravelTimeline.SetLooping(false);
	}
}

void UDashNavModeComponent::Preflight()
{
	Super::Preflight();
	
	InputDirection = GetCharacterMovement()->GetLastInputVector();
	InputDirection = FVector(InputDirection.X, InputDirection.Y, 0.f);
	InputDirection = InputDirection.GetSafeNormal();
	
	if (!bLateralOnly)
	{
		const FVector FrontInput = FModularNavigationUtils::Component(InputDirection, GetCharacter()->GetActorForwardVector());
		const FVector SideInput = InputDirection - FrontInput;
		InputDirection = FrontInput.RotateAngleAxis(GetCharacter()->GetControlRotation().Pitch, GetCharacter()->GetActorForwardVector().Cross(FVector::UpVector)) + SideInput;
	}
}

bool UDashNavModeComponent::CanBegin()
{
	const bool ReqAirControl = GetCharacterMovement()->AirControl >= MinimumAirControlForAirDash;
	
	return CooldownTimer <= 0.f && InputDirection.Length() > UE_KINDA_SMALL_NUMBER && (GetCharacterMovement()->IsMovingOnGround()
		|| (ReqAirControl && AirDashController->HasAirDashes(GetCharacter())));
}

void UDashNavModeComponent::Begin()
{
	Super::Begin();

	bInAirDash = !GetCharacterMovement()->IsMovingOnGround();
	
	if (bInAirDash) AirDashController->Begin(GetCharacter());
	// else GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	LastTravelTimelineValue = 0.f;
	TravelTimeline.PlayFromStart();

	Timer = Duration * (bInAirDash ? AirDurationRatio : 1.f);
	CooldownTimer = Cooldown;
	Direction = InputDirection;
}

void UDashNavModeComponent::Exec(float DeltaTime)
{
	UE_LOG(LogModularNavigation, Log, TEXT("UDashNavModeComponent::Exec; Timer=%f, DeltaTime=%f, LocalRole = %i, RemoteRole = %i"),
		Timer, DeltaTime, GetCharacter()->GetLocalRole(), GetCharacter()->GetRemoteRole());
	
	const bool bUseTravelCurve = bInAirDash ? bUseTravelCurveInAir : bUseTravelCurveOnGround;
	const float CalcDuration = Duration * (bInAirDash ? AirDurationRatio : 1.f);
	if (bUseTravelCurve)
		TravelTimeline.TickTimeline(DeltaTime / CalcDuration);
	else
		GetCharacterMovement()->Velocity = Direction * ((Distance * (bInAirDash ? AirDistanceRatio : 1.f)) / CalcDuration)
			* (bAirControlAffectsAirDashDistance && bInAirDash ? GetCharacterMovement()->AirControl : 1.f);
	
	LastTravelTimelineTime = GetWorld()->TimeSeconds;

	Timer -= DeltaTime;
	
	Super::Exec(DeltaTime);
}

bool UDashNavModeComponent::ShouldEnd()
{
	return Timer <= 0;
} 

void UDashNavModeComponent::End()
{
	Timer = -1;
	Direction = FVector::Zero();

	if (MaxInheritFactor < 1.f)
	{
		const float NewSpeed = FMath::Lerp(GetCharacterMovement()->MaxWalkSpeed,
			GetCharacterMovement()->Velocity.Length(), MaxInheritFactor);
		GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.GetSafeNormal() * NewSpeed;
	}

	if (GetCharacterMovement()->MovementMode == MOVE_Flying) GetCharacterMovement()->SetDefaultMovementMode();
	
	Super::End();
}

void UDashNavModeComponent::Idle(float DeltaTime)
{
	AirDashController->Idle(GetCharacter());
	CooldownTimer -= DeltaTime;
	LastTravelTimelineTime = GetWorld()->TimeSeconds;
}

FString UDashNavModeComponent::GetDebugInfo()
{
	return FString::Format(TEXT("{0}, Timer = {1}, CooldownTimer = {2}, InputDirection = {3}, Direction = {4}"),
		{Super::GetDebugInfo(), Timer, CooldownTimer, InputDirection.ToString(), Direction.ToString()});
}

void UDashNavModeComponent::HandleTravelProgress(float Value)
{
	const float DistanceToTravel = (Value - LastTravelTimelineValue) * (Distance * (bInAirDash ? AirDistanceRatio : 1.f));
	const float DeltaTime = GetWorld()->TimeSeconds - LastTravelTimelineTime;
	const float AirMulti = bAirControlAffectsAirDashDistance && !GetCharacterMovement()->IsMovingOnGround() ? GetCharacterMovement()->AirControl : 1.f;

	const float Speed = (DistanceToTravel / DeltaTime) * AirMulti;
	UE_LOGFMT(LogModularNavigation, Log, "DistanceToTravel = {0}, DeltaTime = {1}, AirMulti = {2}, Speed = {3}",
		DistanceToTravel, DeltaTime, AirMulti, Speed);

	GetCharacterMovement()->Velocity = Direction * Speed;
	
	LastTravelTimelineValue = Value;
}
