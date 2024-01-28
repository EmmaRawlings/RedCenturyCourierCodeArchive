#include "MainCharacterMovement.h"

#include "GameFramework/Character.h"
#include "RedCenturyCourier/Utility/Utils.h"


UMainCharacterMovement::UMainCharacterMovement()
{
	// Defaults
	MaxRush = 0.f;
	PassiveRushGain = 0.f;
	RushTimeout = 0.f;
	RushTimeoutSpeedTolerance = 0.1f;
	PassiveRushLoss = 10.f;
	Rush = 0.f;
	// RushSpeedMultiCurve = CreateDefaultSubobject<UCurveFloat>(TEXT("RushSpeedMultiCurve"));
	// RushTimeMultiCurve = CreateDefaultSubobject<UCurveFloat>(TEXT("RushTimeMultiCurve"));

	// State
	RushTimeoutTimer = 0.f;
}


void UMainCharacterMovement::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MaxRush == 0)
	{
		Rush = 0;
	}
	else
	{
		const FVector InputXY = FVector::VectorPlaneProject(GetLastInputVector(), FVector::UpVector);
		const FVector VelocityXY = FVector::VectorPlaneProject(Velocity, FVector::UpVector);
		const FVector ControlXY = FVector::VectorPlaneProject(GetCharacterOwner()->GetControlRotation().RotateVector(FVector::ForwardVector), FVector::UpVector);

		const bool bRushBelowMax = Rush < MaxRush;
		const bool bCanBuildRush =
			(!VelocityXY.IsNearlyZero(RushTimeoutSpeedTolerance)
			&& FUtils::AngleInDegrees(InputXY,  VelocityXY) < 60.f
			&& FUtils::AngleInDegrees(InputXY,  ControlXY) < 60.f)
			|| IsAnyNavModeActive();
		const bool bCantLoseRush = IsFalling();
		const bool bRushAboveMax = Rush > MaxRush;
		
		if (bRushBelowMax && bCanBuildRush)
		{
			RushTimeoutTimer = 0.f;
			Rush += PassiveRushGain * DeltaTime / GetCharacterOwner()->GetActorTimeDilation();
			Rush = FMath::Clamp(Rush, 0.f, MaxRush);
		}
		else if ((!bCanBuildRush && !bCantLoseRush) || bRushAboveMax)
		{
			RushTimeoutTimer += DeltaTime / GetCharacterOwner()->GetActorTimeDilation();
			if (RushTimeoutTimer >= RushTimeout)
			{
				Rush -= PassiveRushLoss * DeltaTime / GetCharacterOwner()->GetActorTimeDilation();
				Rush = FMath::Clamp(Rush, 0.f, MaxRush);
				RushTimeoutTimer = RushTimeout;
			}
		}
	}

	// // TODO modify GetActorTimeDilation instead?
	// GetCharacterOwner()->CustomTimeDilation = RushTimeMultiCurve->GetFloatValue(Rush);
	// GetWorld()->GetWorldSettings()->SetTimeDilation(RushTimeMultiCurve->GetFloatValue(Rush));
}

// bool UMainCharacterMovement::MoveUpdatedComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep,
//                                                       FHitResult* OutHit, ETeleportType Teleport)
// {
// 	return Super::MoveUpdatedComponentImpl(Delta * RushTimeMultiCurve->GetFloatValue(Rush), NewRotation, bSweep, OutHit, Teleport);
// }

float UMainCharacterMovement::GetMaxSpeed() const
{
	return Super::GetMaxSpeed() * RushSpeedMultiCurve->GetFloatValue(Rush);
}

float UMainCharacterMovement::GetRushTimeDilation()
{
	return RushTimeMultiCurve->GetFloatValue(Rush);
}
