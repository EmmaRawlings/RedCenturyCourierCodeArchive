// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularCharacterMovementComponent.h"

#include "ModularNavigationCharacter.h"
#include "ModularNavigationUtils.h"
#include "NavModeComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Logging/StructuredLog.h"

#pragma region SavedMove

void FSavedModularMove_Character::Clear()
{
	FSavedMove_Character::Clear();

	TArray<uint8> NavModeKeys;
	NavModePilotData.GetKeys(NavModeKeys);
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		NavModePilotData[NavModeKeyEntry]->Clear();
	}
	SavedNavModeKey = 0;
	NavModeSavedData.GetKeys(NavModeKeys);
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		NavModeSavedData[NavModeKeyEntry]->Clear();
	}
}

void FSavedModularMove_Character::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	const UModularCharacterMovementComponent* ModularCharacterMovement = Cast<UModularCharacterMovementComponent>(C->GetCharacterMovement());

	StartNavModeKey = ModularCharacterMovement->NavModeKey;
	
	TArray<uint8> NavModeKeys;
	ModularCharacterMovement->NavModeByKey.GetKeys(NavModeKeys);
	for (uint8 NavModeKeyEntry : NavModeKeys)
	{
		UNavModeComponent* NavMode = ModularCharacterMovement->NavModeByKey[NavModeKeyEntry];
		if (!NavModePilotData.Contains(NavModeKeyEntry))
			NavModePilotData.Add(NavModeKeyEntry, NavMode->NewNetworkPilotData());
		NavModePilotData[NavModeKeyEntry]->RecordPreflight(C, NavMode, InDeltaTime);
	}
}

void FSavedModularMove_Character::PostUpdate(ACharacter* C, EPostUpdateMode PostUpdateMode)
{
	FSavedMove_Character::PostUpdate(C, PostUpdateMode);
	
	const UModularCharacterMovementComponent* ModularCharacterMovement = Cast<UModularCharacterMovementComponent>(C->GetCharacterMovement());

	SavedNavModeKey = ModularCharacterMovement->NavModeKey;
	
	TArray<uint8> NavModeKeys;
	ModularCharacterMovement->NavModeByKey.GetKeys(NavModeKeys);
	for (uint8 NavModeKeyEntry : NavModeKeys)
	{
		UNavModeComponent* NavMode = ModularCharacterMovement->NavModeByKey[NavModeKeyEntry];
		if (!NavModeSavedData.Contains(NavModeKeyEntry))
			NavModeSavedData.Add(NavModeKeyEntry, NavMode->NewNetworkSavedData());
		NavModeSavedData[NavModeKeyEntry]->RecordPostUpdate(C, NavMode, PostUpdateMode);
	}

	if (PostUpdateMode == PostUpdate_Record)
	{
		// Don't combine if this move caused us to change nav modes during the move.
		if (StartNavModeKey != SavedNavModeKey)
		{
			bForceNoCombine = true;
		}
	}
}

bool FSavedModularMove_Character::IsImportantMove(const FSavedMovePtr& LastAckedMovePtr) const
{
	const FSavedModularMove_Character* LastAckedMove = static_cast<const FSavedModularMove_Character*>(LastAckedMovePtr.Get());

	// TODO this is a temporary, overly eager solution
	TArray<uint8> NavModeKeys;
	LastAckedMove->NavModePilotData.GetKeys(NavModeKeys);
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		if (LastAckedMove->NavModePilotData[NavModeKeyEntry]->bIsAllowed)
			return true;
	}
	NavModePilotData.GetKeys(NavModeKeys);
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		if (NavModePilotData[NavModeKeyEntry]->bIsAllowed)
			return true;
	}
	
	return FSavedMove_Character::IsImportantMove(LastAckedMovePtr);
}

bool FSavedModularMove_Character::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter,
	float MaxDelta) const
{
	const FSavedModularMove_Character* NewMove = static_cast<const FSavedModularMove_Character*>(NewMovePtr.Get());

	// TODO combine
	TArray<uint8> NavModeKeys;
	NavModePilotData.GetKeys(NavModeKeys);
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		if (NavModePilotData[NavModeKeyEntry]->bIsAllowed)
			return false;
	}
	NewMove->NavModePilotData.GetKeys(NavModeKeys);
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		if (NewMove->NavModePilotData[NavModeKeyEntry]->bIsAllowed)
			return false;
	}
	
	return FSavedMove_Character::CanCombineWith(NewMovePtr, InCharacter, MaxDelta);
}

FSavedMovePtr FNetworkPredictionData_Client_Character_ModularMovement::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedModularMove_Character());
}

FNetworkPredictionData_Client* UModularCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UModularCharacterMovementComponent* MutableThis = const_cast<UModularCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Character_ModularMovement(*this);
	}

	return ClientPredictionData;
}

FNetworkPredictionData_Server* UModularCharacterMovementComponent::GetPredictionData_Server() const
{
	if (ServerPredictionData == nullptr)
	{
		UModularCharacterMovementComponent* MutableThis = const_cast<UModularCharacterMovementComponent*>(this);
		MutableThis->ServerPredictionData = new FNetworkPredictionData_Server_ModularCharacter(*this);
	}

	return ServerPredictionData;
}

#pragma endregion SavedMove

UModularCharacterMovementComponent::UModularCharacterMovementComponent()
{
	// Defaults
	bAllowWalkOntoSteepSlope = true;
	WalkOntoSteepSlopeVelocityScale = 0.5;
	WalkOntoSteepSlopeMaxAngleDelta = 20;
	bExecNavModes = true;
	InAirStepMaxHeight = 40.f;
	InAirStepMaxDist = 5.f;
	InAirStepMaxVerticalSpeed = 200.f;
	bInAirStepResetVerticalSpeed = true;
	SmoothCrouchedStateThreshold = 0.f;
	SmoothUnCrouchedStateThreshold = 1.f;
	// CrouchCurve = CreateDefaultSubobject<UCurveFloat>(TEXT("CrouchCurve"));

	// Nav mode
	NavModeKey = 0;
	
	// Network
	SetNetworkMoveDataContainer(NetworkMoveDataContainer);
	SetMoveResponseDataContainer(MoveResponseDataContainer);
}

void UModularCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	MapOutNavModesOnBeginPlay();

	// if (CrouchCurve)
	// {
	// 	FOnTimelineFloat CrouchProgressFunction;
	// 	CrouchProgressFunction.BindUFunction(this, FName("HandleCrouchProgress"));
	// 	CrouchTimeline.AddInterpFloat(CrouchCurve, CrouchProgressFunction);
	// 	CrouchTimeline.SetLooping(false);
	// }
}

void UModularCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	LastSurfaceNormal = &this->CurrentFloor.HitResult.Normal;
	
	if (bUseSmoothCrouch)
		ProcessSmoothCrouch(DeltaTime);
}

#pragma region NavigationModes

void UModularCharacterMovementComponent::MapOutNavModesOnBeginPlay()
{
	// Generate a key for each nav mode for network replication purposes
	const ACharacter* Character = GetCharacterOwner();
	TArray<UNavModeComponent*> NavModes;
	Character->GetComponents<UNavModeComponent*>(NavModes);
	// Sort alphabetically, so that the keys are consistent for server & client
	NavModes.Sort([Character](const UNavModeComponent& a, const UNavModeComponent& b)
	{
		const int32 IntComp = a.GetFName().Compare(b.GetFName());
		if (IntComp == 0)
			UE_LOG(LogTemp, Warning, TEXT("One or more NavigationModes have the same name, %s, for actor, %s."),
				*a.GetFName().ToString(), *Character->GetFName().ToString());
		return IntComp < 0;
	});
	
	uint8 NavModeKeyIncrement = 1; // 0 = no nav mode
	for (UNavModeComponent* NavMode : NavModes)
	{
		NavModeByKey.Add(NavModeKeyIncrement, NavMode);
		KeyByNavMode.Add(NavMode, NavModeKeyIncrement);
		NavModeKeyIncrement++;
	}
}

void UModularCharacterMovementComponent::SwitchNavMode(UNavModeComponent* NavModeComponent)
{
	if (!bExecNavModes) return;
	
	if (NavModeKey == 0 && NavModeComponent == nullptr
		|| NavModeKey == (NavModeComponent == nullptr ? 0 : KeyByNavMode[NavModeComponent]))
			return;
	
	// End current mode
	if (NavModeKey != 0)
	{
		NavModeByKey[NavModeKey]->End();
	}

	// Switch and begin new mode
	if (NavModeComponent == nullptr)
	{
		NavModeKey = 0;
		return;
	}
	NavModeKey = KeyByNavMode[NavModeComponent];
	NavModeComponent->Begin();
}

bool UModularCharacterMovementComponent::IsAnyNavModeActive()
{
	return NavModeKey != 0;
}

bool UModularCharacterMovementComponent::IsNavModeActive(const UNavModeComponent* NavModeComponent)
{
	return IsAnyNavModeActive() && KeyByNavMode[NavModeComponent] == NavModeKey;
}

bool UModularCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
	if (NavModeKey != 0 && NavModeByKey[NavModeKey]->DoJump(bReplayingMoves))
		return true;
	
	return Super::DoJump(bReplayingMoves);
}

bool UModularCharacterMovementComponent::NavModeAllowsFirstAirJump()
{
	return NavModeKey != 0 && NavModeByKey[NavModeKey]->AllowsFirstAirJump();
}

#pragma endregion NavigationModes

#pragma region SmoothCrouch

void UModularCharacterMovementComponent::Crouch(bool bClientSimulation)
{
	const ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	UnCrouchedHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	
	// Super::Crouch(bClientSimulation);

	// TODO smooth crouch replication bCrouchIsClientSimulation = bClientSimulation;

	// TODO can't interupt uncrouch if we haven't hit the uncrouch threshold (doesn't affect current project settings)
	
	bCrouchIntent = true;
	if (!bUseSmoothCrouch) {
		HandleCrouchProgress(1.f);
	}
}

void UModularCharacterMovementComponent::UnCrouch(bool bClientSimulation)
{
	// Super::UnCrouch(bClientSimulation);
	
	// TODO smooth crouch replication bCrouchIsClientSimulation = bClientSimulation;

	// TODO can't interupt crouch if we haven't hit the crouch threshold (doesn't affect current project settings)
	
	bCrouchIntent = false;
	if (!bUseSmoothCrouch)
	{
		HandleCrouchProgress(0.f);
	}
}

const auto CalcSmoothCrouchAlpha = [](const UCurveFloat* CrouchCurve, float Alpha)
{
	return FMath::Clamp(CrouchCurve->GetFloatValue(Alpha), 0.f, 1.f);
};

void UModularCharacterMovementComponent::ProcessSmoothCrouch(float DeltaTime)
{
	const float ProposedSmoothCrouchTime = FMath::Clamp(
		SmoothCrouchTime + (bCrouchIntent ? DeltaTime : -DeltaTime),
		0.f,
		CrouchCurve->FloatCurve.GetLastKey().Time);
	if (ProposedSmoothCrouchTime != SmoothCrouchTime && HandleCrouchProgress(CalcSmoothCrouchAlpha(CrouchCurve, ProposedSmoothCrouchTime)))
	{
		SmoothCrouchTime = ProposedSmoothCrouchTime;
	}
}

bool UModularCharacterMovementComponent::HandleCrouchProgress(float NewAlpha)
{
	if (!HasValidData())
		return false;
	if (/* TODO smooth crouch replication !bCrouchIsClientSimulation && */!CanCrouchInCurrentState())
		return false;
	
	const float Delta = NewAlpha - CrouchAlpha;

	if (Delta == 0)
	{
		// Commit changes
		// UE_LOG(LogTemp, Log, TEXT("Crouch handled with no change required."));
		return true;
	}
	
	const auto CalcProposedNewHalfHeight = [this](float UnscaledRadius, float ProposedCrouchProgress)
	{
		// Height is not allowed to be smaller than radius.
		return FMath::Max3(0.f, UnscaledRadius,
			FMath::Lerp(UnCrouchedHalfHeight, GetCrouchedHalfHeight(), ProposedCrouchProgress));
	};

	const float UnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	const float ScaledCurrentHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float ProposedHalfHeight = CalcProposedNewHalfHeight(UnscaledRadius, NewAlpha);
	const float ScaledProposedHalfHeight = ProposedHalfHeight * CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	//const float HalfHeightAdjust = (UnCrouchedHalfHeight - ProposedHalfHeight);
	const float ScaledHalfHeightAdjust = ScaledProposedHalfHeight - ScaledCurrentHalfHeight;

	// As per epic's implementation, bCrouchMaintainsBaseLocation is set true when on ground and false otherwise;
	//	however, this behaviour could be overridden. It's optimal to maintain base if on ground as the encroaching
	//	adjustment code further down would simply do this anyway (incurring sweep checks in the process).
	const bool bMaintainingBaseLocation = bCrouchMaintainsBaseLocation || IsMovingOnGround();
	float ZPosAdjust = bMaintainingBaseLocation ? ScaledHalfHeightAdjust : 0.f;

	// if capsule height is increasing, then sweep for ceiling
	if (ScaledHalfHeightAdjust > 0)
	{
		// Try to stay in place and see if the larger capsule fits. We use a slightly taller capsule to avoid penetration.
		const UWorld* MyWorld = GetWorld();
		const float SweepInflation = UE_KINDA_SMALL_NUMBER * 10.f;
		FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);

		// Compensate for the difference between current capsule size and standing size
		const FCollisionShape StandingCapsuleShape =
			// Shrink by negative amount, so actually grow it.
			GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust);
		const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();

		const auto IsEncroached = [this, MyWorld, ZPosAdjust, StandingCapsuleShape, CapsuleParams, ResponseParam]()
		{
			return MyWorld->OverlapBlockingTestByChannel(
			UpdatedComponent->GetComponentLocation() + FVector(0.f,0.f,ZPosAdjust),
			FQuat::Identity, UpdatedComponent->GetCollisionObjectType(),
			StandingCapsuleShape, CapsuleParams, ResponseParam);
		};
		
		// If encroached, attempt adjustment, or cancel if no room
		if( IsEncroached() )
		{
			// sweep constants
			float PawnRadius, PawnHalfHeight;
			CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
			const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
			const float TraceDist = PawnRadius + ScaledHalfHeightAdjust;
			const FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, ShrinkHalfHeight);

			const auto SweepAdjust = [this, MyWorld, CollisionChannel, ShortCapsuleShape, CapsuleParams, TraceDist](float DistMulti, float& ZPosAdjust)
			{
				const FVector StartToEnd = FVector(0.f, 0.f, DistMulti * TraceDist);
				FHitResult Hit;
				if (MyWorld->SweepSingleByChannel(Hit, UpdatedComponent->GetComponentLocation(),
					UpdatedComponent->GetComponentLocation() + StartToEnd, FQuat::Identity, CollisionChannel, ShortCapsuleShape, CapsuleParams))
				{
					ZPosAdjust = -1 * DistMulti * (TraceDist - Hit.Distance);
					return true;
				}
				return false;
			};

			// adjust upwards up in case of floor (we could be in air just above floor)
			bool bFloorHit = false;
			if (!bMaintainingBaseLocation)
			{
				bFloorHit = SweepAdjust(-1, ZPosAdjust);
			}

			// adjust downwards in case of ceiling
			if (!bFloorHit)
			{
				SweepAdjust(+1, ZPosAdjust);
			}

			if (IsEncroached())
				return false;
		}
	}

	// Commit changes
	// UE_LOG(LogTemp, Log, TEXT("Crouch handled with cimmited changes: CrouchAlpha = %f, HalfHeight = %f, ZPosAdjust = %f"),
	// 	NewAlpha, ProposedHalfHeight, ZPosAdjust);

	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(UnscaledRadius, ProposedHalfHeight, true);
	Cast<AModularNavigationCharacter>(GetCharacterOwner())
		->CapsuleHeadRoot
		->SetRelativeLocation(FVector(0.f, 0.f, ProposedHalfHeight - UnCrouchedHalfHeight));
	if (bCrouchMaintainsBaseLocation)
	{
		// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of
		//	the capsule from staying at the same spot.
		UpdatedComponent->MoveComponent(FVector(0.f, 0.f, ZPosAdjust),
			UpdatedComponent->GetComponentQuat(), true, nullptr,
			EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	}
	
	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OverallHalfHeightAdjust = GetCrouchedHalfHeight() - UnCrouchedHalfHeight;
	const float ScaledOverallHalfHeightAdjust = OverallHalfHeightAdjust * ComponentScale;
	if (Delta > 0.f && !CharacterOwner->bIsCrouched && (!bUseSmoothCrouch || NewAlpha >= SmoothCrouchedStateThreshold))
	{
		CharacterOwner->bIsCrouched = true;
		CharacterOwner->OnStartCrouch( -OverallHalfHeightAdjust, -ScaledOverallHalfHeightAdjust );
	}
	else if (Delta < 0.f && CharacterOwner->bIsCrouched && (!bUseSmoothCrouch || NewAlpha <= SmoothUnCrouchedStateThreshold))
	{
		CharacterOwner->bIsCrouched = false;
		CharacterOwner->OnEndCrouch( OverallHalfHeightAdjust, ScaledOverallHalfHeightAdjust );
	}

	bForceNextFloorCheck = true;
	
	CrouchAlpha = NewAlpha;

	return true;
}

#pragma endregion SmoothCrouch

#pragma region MovementParameterContext

void UModularCharacterMovementComponent::UseMovementParameterContext(
	FMovementParameterContext InMovementParameterContext)
{
	bMovementParameterOverride = true;
	MovementParameterContext = InMovementParameterContext;
}

void UModularCharacterMovementComponent::UnsetMovementParameterContext()
{
	bMovementParameterOverride = false;
}

FMovementParameterContext UModularCharacterMovementComponent::NeutralContext() const
{
	FMovementParameterContext Context;
	Context.bUseSeparateBrakingFriction = bUseSeparateBrakingFriction;
	Context.MaxAcceleration = MaxAcceleration;
	Context.MaxBrakingDeceleration = BrakingDecelerationWalking;
	Context.BrakingFriction = BrakingFriction;
	Context.BrakingFrictionFactor = BrakingFrictionFactor;
	Context.GroundFriction = GroundFriction;
	Context.bAllowResetJumpState = true;
	Context.MaxSpeed = MaxWalkSpeed;
	Context.WalkOntoSteepSlopeVelocityScale = WalkOntoSteepSlopeVelocityScale;
	return Context;
}

FMovementParameterContext UModularCharacterMovementComponent::GetMovementParameterContext() const
{
	return MovementParameterContext;
}

float UModularCharacterMovementComponent::GetMaxSpeed() const
{
	if (bMovementParameterOverride)
		return GetMovementParameterContext().MaxSpeed;

	switch(MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		if (bUseSmoothCrouch)
			return FMath::Lerp(MaxWalkSpeed, MaxWalkSpeedCrouched,
				bUseSmoothCrouch ? CalcSmoothCrouchAlpha(CrouchCurve, SmoothCrouchTime) : 1.f);
	default:
		return Super::GetMaxSpeed();
	}
}

float UModularCharacterMovementComponent::GetWalkOntoSteepSlopeVelocityScale() const
{
	if (bMovementParameterOverride)
		return GetMovementParameterContext().WalkOntoSteepSlopeVelocityScale;
	
	return WalkOntoSteepSlopeVelocityScale;
}

void UModularCharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid,
	float BrakingDeceleration)
{
	const bool UseContext = bMovementParameterOverride;
	const bool NormalBUseSeparateBrakingFriction = bUseSeparateBrakingFriction;
	const float NormalBrakingFriction = BrakingFriction;
	if (UseContext)
	{
		bUseSeparateBrakingFriction = GetMovementParameterContext().bUseSeparateBrakingFriction;
		BrakingFriction = GetMovementParameterContext().BrakingFriction;
	}
	
	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
	
	if (UseContext)
	{
		bUseSeparateBrakingFriction = NormalBUseSeparateBrakingFriction;
		BrakingFriction = NormalBrakingFriction;
	}
}

float UModularCharacterMovementComponent::GetMaxAcceleration() const
{
	return bMovementParameterOverride ? GetMovementParameterContext().MaxAcceleration : Super::GetMaxAcceleration();
}

float UModularCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	return bMovementParameterOverride ? GetMovementParameterContext().MaxBrakingDeceleration : Super::GetMaxBrakingDeceleration();
}

void UModularCharacterMovementComponent::ApplyVelocityBraking(float DeltaTime, float Friction,
	float BrakingDeceleration)
{
	const bool UseContext = bMovementParameterOverride;
	const float NormalBrakingFrictionFactor = BrakingFrictionFactor;
	if (UseContext)
	{
		BrakingFrictionFactor = GetMovementParameterContext().BrakingFrictionFactor;
	}
	
	Super::ApplyVelocityBraking(DeltaTime, Friction, BrakingDeceleration);
	
	if (UseContext)
	{
		BrakingFrictionFactor = NormalBrakingFrictionFactor;
	}
}

void UModularCharacterMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	const bool UseContext = bMovementParameterOverride;
	const float NormalGroundFriction = GroundFriction;
	if (UseContext)
	{
		GroundFriction = GetMovementParameterContext().GroundFriction;
	}
	
	Super::PhysWalking(deltaTime, Iterations);
	
	if (UseContext)
	{
		GroundFriction = NormalGroundFriction;
	}
}

void UModularCharacterMovementComponent::PhysNavWalking(float deltaTime, int32 Iterations)
{
	const bool UseContext = bMovementParameterOverride;
	const float NormalGroundFriction = GroundFriction;
	if (UseContext)
	{
		GroundFriction = GetMovementParameterContext().GroundFriction;
	}
	
	Super::PhysNavWalking(deltaTime, Iterations);
	
	if (UseContext)
	{
		GroundFriction = NormalGroundFriction;
	}
}

#pragma endregion MovementParameterContext

#pragma region DampenAirControl

// TODO
// float UModularCharacterMovementComponent::GetCurrentAirControl()
// {
// 	if (bMovementParameterOverride)
// 		return DampenedAirControl;
// 	else
// 		return AirControl;
// }
//
// FVector UModularCharacterMovementComponent::GetFallingLateralAcceleration(float DeltaTime)
// {
// 	const bool UseContext = bMovementParameterOverride;
// 	const float NormalAirControl = AirControl;
// 	if (UseContext)
// 	{
// 		AirControl = DampenedAirControl;
// 	}
// 	
// 	const FVector Result = Super::GetFallingLateralAcceleration(DeltaTime);
// 	
// 	if (UseContext)
// 	{
// 		AirControl = NormalAirControl;
// 	}
// 	
// 	return Result;
// }

void UModularCharacterMovementComponent::DampenAirControl(float DampenedAirControl, float RecoveryTime)
{
	AirControlDampenInfo.NormalAirControl = FMath::Max(AirControl, AirControlDampenInfo.NormalAirControl);
	AirControlDampenInfo.RecoveryPerSecond = FMath::Max(AirControl - DampenedAirControl / RecoveryTime, 0.f);
	if (DampenedAirControl > AirControlDampenInfo.NormalAirControl)
		return;
	AirControl = DampenedAirControl;
}

void UModularCharacterMovementComponent::ResetAirControlDampening()
{
	if (AirControl < AirControlDampenInfo.NormalAirControl)
		AirControl = AirControlDampenInfo.NormalAirControl;
}

void UModularCharacterMovementComponent::StartNewPhysics(float deltaTime, int32 Iterations)
{
	if (AirControl < AirControlDampenInfo.NormalAirControl)
	{
		switch ( MovementMode )
		{
		case MOVE_Walking:
		case MOVE_NavWalking:
		case MOVE_Flying:
			AirControl = AirControlDampenInfo.NormalAirControl;
			break;
		case MOVE_None:
		case MOVE_Swimming:
		case MOVE_Falling:
		case MOVE_Custom:
		default:
			AirControl += AirControlDampenInfo.RecoveryPerSecond * deltaTime;
			break;
		}
	}

	Super::StartNewPhysics(deltaTime, Iterations);
}

#pragma endregion DampenAirControl

#pragma region AllowWalkOntoSteepSlope

float UModularCharacterMovementComponent::SlideAlongSurface(const FVector& Delta, float Time, const FVector& InNormal,
	FHitResult& Hit, bool bHandleImpact)
{
	if (bAllowWalkOntoSteepSlope && LastSurfaceNormal != nullptr
		&& FModularNavigationUtils::AngleInDegrees(InNormal, *LastSurfaceNormal) <= WalkOntoSteepSlopeMaxAngleDelta)
	{
		if (Delta.Length() > 0.f)
		{
			const FQuat Rotation = UpdatedComponent->GetComponentQuat();
			SafeMoveUpdatedComponent(Delta, Rotation, true, Hit);
		}
		SetMovementMode(MOVE_Falling, 0);

		FHitResult MaxRampHit = FHitResult();
		MaxRampHit.Normal = FVector::ForwardVector.RotateAngleAxis(GetWalkableFloorAngle(), FVector::RightVector);
		MaxRampHit.ImpactNormal = MaxRampHit.Normal;
		const float MaximumSpeed = FModularNavigationUtils::ComputeGroundMovement(FVector::ForwardVector * Velocity.Length(), MaxRampHit, bMaintainHorizontalGroundVelocity).Length();
		Velocity = FModularNavigationUtils::ComputeGroundMovement(Velocity, Hit, bMaintainHorizontalGroundVelocity);
		if (Velocity.Length() > MaximumSpeed)
		{
			Velocity = Velocity.GetSafeNormal() * MaximumSpeed;
		}

		if (GetWalkOntoSteepSlopeVelocityScale() >= 0)
		{
			Velocity *= GetWalkOntoSteepSlopeVelocityScale();
		}
		
		return 0.f;
	} else {
		return Super::SlideAlongSurface(Delta, Time, InNormal, Hit, bHandleImpact);
	}
}

#pragma endregion AllowWalkOntoSteepSlope

#pragma region InAirStep

void UModularCharacterMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
	const auto DetectLedgeToStep = [this](TArray<FHitResult>& OutHits)
	{
		const UCapsuleComponent* CapsuleComponent = GetCharacterOwner()->GetCapsuleComponent();
		TArray<AActor*> ActorsToIgnore = TArray<AActor*>();
		ActorsToIgnore.Add(GetCharacterOwner());
		const FVector LateralOffset = FVector(Velocity.X, Velocity.Y, 0.f).GetSafeNormal() * InAirStepMaxDist;
		const FVector Start = LateralOffset + CapsuleComponent->GetComponentLocation() + FVector::UpVector * InAirStepMaxHeight;
		const FVector End = LateralOffset + CapsuleComponent->GetComponentLocation();
		const bool IsLedgeHit = UKismetSystemLibrary::CapsuleTraceMulti(GetWorld(),
																		Start,
																		End,
																		CapsuleComponent->GetScaledCapsuleRadius(),
																		CapsuleComponent->GetScaledCapsuleHalfHeight(),
																		ETraceTypeQuery::TraceTypeQuery1,
																		false,
																		ActorsToIgnore,
																		EDrawDebugTrace::None,
																		OutHits,
																		true);

		// UE_LOG(LogModularNavigation, Log, TEXT("UModularCharacterMovementComponent::PhysFalling - checking for in-air ledge to step up. IsLedgeHit = %i, IsValidBlockingHit = %i, AngleInDegrees = %f, GetWalkableFloorAngle() = %f"),
		// 	IsLedgeHit, IsLedgeHit ? OutHits[0].IsValidBlockingHit() : -1,
		// 	IsLedgeHit ? FModularNavigationUtils::AngleInDegrees(OutHits[0].ImpactNormal, FVector::UpVector) : -1.f,
		// 	GetWalkableFloorAngle());
		return IsLedgeHit
			&& OutHits[0].IsValidBlockingHit()
			&& FModularNavigationUtils::AngleInDegrees(OutHits[0].ImpactNormal, FVector::UpVector) <= GetWalkableFloorAngle();
	};

	if (TArray<FHitResult> OutHits;
		!IsMovingOnGround()
		&& (Velocity.X != 0.f || Velocity.Y != 0.f)
		&& Velocity.Z <= InAirStepMaxVerticalSpeed
		&& (NavModeKey == 0 || NavModeByKey[NavModeKey]->AllowsAirLedgeStep())
		&& DetectLedgeToStep(OutHits))
	{
		// UE_LOG(LogModularNavigation, Log, TEXT("UModularCharacterMovementComponent::PhysFalling - doing in-air ledge step up"));
		GetCharacterOwner()->SetActorLocation(GetCharacterOwner()->GetActorLocation()
			+ FVector::UpVector * (OutHits[0].Location.Z - GetCharacterOwner()->GetActorLocation().Z));
		if (bInAirStepResetVerticalSpeed) Velocity.Z = 0.f;
	}
	
	Super::PhysFalling(deltaTime, Iterations);
}

#pragma endregion InAirStep

#pragma region JumpWhilstCrouched

bool UModularCharacterMovementComponent::CanAttemptJump() const
{
	return IsJumpAllowed() &&
		   (Cast<AModularNavigationCharacter>(GetCharacterOwner())->bCanJumpWhilstCrouched || !bWantsToCrouch) &&
		   (IsMovingOnGround() || IsFalling()); // Falling included for double-jump and non-zero jump hold time, but validated by character.
}

#pragma endregion JumpWhilstCrouched

void UModularCharacterMovementComponent::ControlledCharacterMove(const FVector& InputVector, float DeltaSeconds)
{
	TArray<uint8> NavModeKeys;
	NavModeByKey.GetKeys(NavModeKeys);
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		NavModeByKey[NavModeKeyEntry]->Preflight();
	}
	
	Super::ControlledCharacterMove(InputVector, DeltaSeconds);
}

void UModularCharacterMovementComponent::MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags,
	const FVector& NewAccel)
{
	//const bool bIsRemoteClient = GetOwner()->GetLocalRole() == ROLE_Authority && GetOwner()->GetRemoteRole() == ROLE_AutonomousProxy;
	// const bool bIsLocalClient = GetOwner()->GetLocalRole() == ROLE_AutonomousProxy && GetOwner()->GetRemoteRole() == ROLE_Authority;
	// if (bIsRemoteClient)
	// {
	const FSavedModularMove_Character* ReplayData = static_cast<const FSavedModularMove_Character*>(GetCurrentReplayedSavedMove());
	const FCharacterNetworkModularMoveData* NetworkData = static_cast<FCharacterNetworkModularMoveData*>(GetCurrentNetworkMoveData());
	TArray<uint8> NavModeKeys;
	NavModeByKey.GetKeys(NavModeKeys);
	if (ReplayData) {
		for (const uint8 NavModeKeyEntry : NavModeKeys)
		{
			ReplayData->NavModePilotData[NavModeKeyEntry]->Fill(GetCharacterOwner(), NavModeByKey[NavModeKeyEntry]);
		}
	} else if (NetworkData) {
		for (const uint8 NavModeKeyEntry : NavModeKeys)
		{
			NetworkData->NavModePilotData[NavModeKeyEntry]->Fill(GetCharacterOwner(), NavModeByKey[NavModeKeyEntry]);
		}
	}
	//}
	
	Super::MoveAutonomous(ClientTimeStamp, DeltaTime, CompressedFlags, NewAccel);
}

void UModularCharacterMovementComponent::PerformMovement(float DeltaTime)
{
	// UE_LOG(LogModularNavigation, Log, TEXT("UModularCharacterMovementComponent::PerformMovement: DeltaTime = %f, LocalRole = %i, RemoteRole = %i"),
	// 	DeltaTime, GetCharacterOwner()->GetLocalRole(), GetCharacterOwner()->GetRemoteRole());
	const auto PerformNavModes = [this, DeltaTime]
	{
		TArray<UNavModeComponent*> NavModes;
		KeyByNavMode.GetKeys(NavModes);

		for (UNavModeComponent* NavMode : NavModes)
		{
			if (KeyByNavMode[NavMode] == NavModeKey)
			{
				NavMode->Exec(DeltaTime);
			}
			else
			{
				NavMode->Idle(DeltaTime);
			}
		}
	};

	const auto SwitchNavModeIfAble = [this]
	{
		TArray<UNavModeComponent*> NavModes;
		KeyByNavMode.GetKeys(NavModes);

		for (UNavModeComponent* NavMode : NavModes)
		{
			if (KeyByNavMode[NavMode] == NavModeKey)
			{
				if (!NavModeByKey[NavModeKey]->IsAllowed() || NavModeByKey[NavModeKey]->ShouldEnd())
					SwitchNavMode(nullptr);
				continue;
			}
			
			if (NavMode->IsAllowed() && NavMode->CanBegin())
			{
				SwitchNavMode(NavMode);
				// UE_LOGFMT(LogModularNavigation, Log, "Active Nav Mode details: [{0}]", *NavMode->GetDebugInfo());
				break;
			}
		}
	};
	
	if (bExecNavModes) SwitchNavModeIfAble();
	
	if (bExecNavModes) PerformNavModes();
	
	Super::PerformMovement(DeltaTime);
}

bool UModularCharacterMovementComponent::ServerCheckClientError(float ClientTimeStamp, float DeltaTime,
	const FVector& Accel, const FVector& ClientWorldLocation, const FVector& RelativeClientLocation,
	UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	if (Super::ServerCheckClientError(ClientTimeStamp, DeltaTime, Accel, ClientWorldLocation, RelativeClientLocation,
	                                     ClientMovementBase, ClientBaseBoneName, ClientMovementMode))
		return true;

	const FCharacterNetworkModularMoveData* ServerData = static_cast<FCharacterNetworkModularMoveData*>(GetCurrentNetworkMoveData());
	if (NavModeKey != ServerData->SavedNavModeKey)
		return true;

	TArray<uint8> NavModeKeys;
	NavModeByKey.GetKeys(NavModeKeys);
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		if (ServerData->NavModeSavedData[NavModeKeyEntry]->CheckForError(GetCharacterOwner(), NavModeByKey[NavModeKeyEntry]))
			return true;
	}
	
	return false;
}

void UModularCharacterMovementComponent::ServerMoveHandleClientError(float ClientTimeStamp, float DeltaTime,
                                                                     const FVector& Accel, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase,
                                                                     FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	Super::ServerMoveHandleClientError(ClientTimeStamp, DeltaTime, Accel, RelativeClientLocation, ClientMovementBase,
	                                   ClientBaseBoneName, ClientMovementMode);
	
	FNetworkPredictionData_Server_ModularCharacter* ServerData =
		static_cast<FNetworkPredictionData_Server_ModularCharacter*>(GetPredictionData_Server_Character());
	check(ServerData);
	if (!ServerData->PendingAdjustment.bAckGoodMove)
	{
		ServerData->PendingAdjustmentNavModeKey = NavModeKey;
		TArray<uint8> NavModeKeys;
		NavModeByKey.GetKeys(NavModeKeys);
		for (const uint8 NavModeKeyEntry : NavModeKeys)
		{
			UNavModeComponent* NavMode = NavModeByKey[NavModeKeyEntry];
			if (!ServerData->PendingAdjustmentNavModeData.Contains(NavModeKeyEntry))
				ServerData->PendingAdjustmentNavModeData.Add(NavModeKeyEntry, NavMode->NewNetworkSavedData());
			ServerData->PendingAdjustmentNavModeData[NavModeKeyEntry]
				->RecordPostUpdate(GetCharacterOwner(), NavMode, FSavedMove_Character::PostUpdate_Record);
		}
	}
}

bool UModularCharacterMovementComponent::ClientUpdatePositionAfterServerUpdate()
{
	FNetworkPredictionData_Client_Character_ModularMovement* ClientData =
		static_cast<FNetworkPredictionData_Client_Character_ModularMovement*>(GetPredictionData_Client());
	const FSavedMovePtr RestoreMovePtr = ClientData->AllocateNewMove();
	FSavedModularMove_Character* RestoreMove = static_cast<FSavedModularMove_Character*>(RestoreMovePtr.Get());
	RestoreMove->SetMoveFor(GetCharacterOwner(), 0.f, FVector::Zero(), *ClientData);
	
	const bool bResult = Super::ClientUpdatePositionAfterServerUpdate();
	
	TArray<uint8> NavModeKeys;
	NavModeByKey.GetKeys(NavModeKeys);
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		RestoreMove->NavModePilotData[NavModeKeyEntry]->Fill(GetCharacterOwner(), NavModeByKey[NavModeKeyEntry]);
	}
	
	return bResult;
}

void UModularCharacterMovementComponent::ClientAdjustPosition_Implementation(float TimeStamp, FVector NewLoc,
	FVector NewVel, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition,
	uint8 ServerMovementMode, TOptional<FRotator> OptionalRotation)
{
	Super::ClientAdjustPosition_Implementation(TimeStamp, NewLoc, NewVel, NewBase, NewBaseBoneName, bHasBase,
	                                           bBaseRelativePosition, ServerMovementMode, OptionalRotation);
	const FCharacterModularMoveResponseDataContainer& ResponseDataContainer =
		static_cast<FCharacterModularMoveResponseDataContainer&>(GetMoveResponseDataContainer());

	SwitchNavMode(ResponseDataContainer.ClientAdjustmentNavModeKey == 0 ? nullptr : NavModeByKey[ResponseDataContainer.ClientAdjustmentNavModeKey]);
	
	TArray<uint8> NavModeKeys;
	ResponseDataContainer.ClientAdjustmentNavModeData.GetKeys(NavModeKeys);
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		ResponseDataContainer.ClientAdjustmentNavModeData[NavModeKeyEntry]
			->Fill(GetCharacterOwner(), NavModeByKey[NavModeKeyEntry]);
	}
}
