#include "Utils.h"

FRotator FUtils::RandomSpread(FRotator Direction, float Spread)
{
	// TODO
	check(false);
	return FRotator::ZeroRotator;
	//return Quaternion.AngleAxis(Random.Range(0f, 360f), direction)
	//	* Vector3.RotateTowards(
	//		direction, Vector3.Cross(direction, Vector3.up),
	//		Mathf.Deg2Rad * Random.Range(0f, spread / 2f),
	//		1f
	//	);
}

float FUtils::AngleInDegrees(const FVector& U, const FVector& V)
{
	return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(U.GetSafeNormal(), V.GetSafeNormal())));
}

FVector FUtils::Component(const FVector& U, const FVector& Direction)
{
	return FVector::DotProduct(U.GetSafeNormal(), Direction.GetSafeNormal())
		* Direction.GetSafeNormal()
		* U.Length();
}

// FVector FUtils::ProjectToPlane(FVector U, FVector Normal, bool IncreaseOnly)
// {
// 	const FVector ProjectDirection = FVector::CrossProduct(FVector::CrossProduct(U, Normal), Normal);
// 	FVector Result = ProjectDirection *
// 		(FVector::DotProduct(U, ProjectDirection) / FVector::DotProduct(ProjectDirection, ProjectDirection));
// 	if (IncreaseOnly)
// 	{
// 		Result *= 
// 	}
// }

FVector FUtils::Apex(const FVector& Velocity, const FVector& Acceleration)
{
	if (AngleInDegrees(Velocity,Acceleration) <= 90) return FVector::ZeroVector;
	const FVector U = Component(Velocity, Acceleration);
	return -FMath::Square(U) / (2 * Acceleration);
}

float FUtils::VelocityToReachApex(float Apex, float Acceleration)
{
	return FMath::Sqrt(-2 * Acceleration * Apex);
}

float FUtils::LimitImpulse(float SpeedChange, float Speed, float MaxSpeed)
{
	return FMath::Min(SpeedChange, FMath::Max(0.f, MaxSpeed - Speed));
}

float FUtils::Adjustment(float Current, float AdjustTo, float DeltaTime, float AdjustmentSpeed)
{
	if (AdjustmentSpeed == 0) return 0.f;
	const bool Positive = AdjustmentSpeed > 0;
	if (Positive ? Current >= AdjustTo : Current <= AdjustTo) return 0.f;
	return Positive
		? FMath::Min(AdjustmentSpeed * DeltaTime, AdjustTo - Current)
		: FMath::Max(AdjustmentSpeed * DeltaTime, AdjustTo - Current);
}


FVector FUtils::ComputeGroundMovement(const FVector Movement, const FHitResult& Hit, bool bMaintainHorizontalGroundVelocity)
{
	// code from UCharacterMovementComponent::ComputeGroundMovementDelta
	const FVector FloorNormal = Hit.ImpactNormal;
	const FVector ContactNormal = Hit.Normal;

	if (FloorNormal.Z < (1.f - UE_KINDA_SMALL_NUMBER) && FloorNormal.Z > UE_KINDA_SMALL_NUMBER && ContactNormal.Z > UE_KINDA_SMALL_NUMBER)
	{
		// Compute a vector that moves parallel to the surface, by projecting the horizontal movement direction onto the ramp.
		const float FloorDotDelta = (FloorNormal | Movement);
		const FVector RampVelocity(Movement.X, Movement.Y, -FloorDotDelta / FloorNormal.Z);
		
		if (bMaintainHorizontalGroundVelocity)
		{
			return RampVelocity;
		}
		else
		{
			return RampVelocity.GetSafeNormal() * Movement.Size();
		}
	}

	return Movement;
}

void FUtils::SetActorActive(AActor* Actor, bool inActive)
{
	if (!Actor) return;
	// TODO there may be situations where this is not the appropriate way to activate/deactivate certain actors
	//      could solve this by defining an ActivatableActor interface
	Actor->SetActorHiddenInGame(!inActive);
	Actor->SetActorTickEnabled(inActive);
	Actor->SetActorEnableCollision(inActive);
	
	TArray<UActorComponent*> Components;
	Actor->GetComponents<UActorComponent*>(Components);
	
	for (UActorComponent* Component : Components)
	{
		if (inActive) Component->Activate(); else Component->Deactivate();
	}

	// TODO saw this: https://www.reddit.com/r/unrealengine/comments/bprbzr/how_to_properly_disable_actor_in_multiplayer/
	//      do we need to do this?
	// UPrimitiveComponent* PrimitiveComponent = TargetActor->FindComponentByClass<UPrimitiveComponent>();
	// if ( PrimitiveComponent )
	// {
	// 	PrimitiveComponent->SetSimulatePhysics( true );
	// 	PrimitiveComponent->SetHiddenInGame( false );
	// 	PrimitiveComponent->SetComponentTickEnabled( true );
	// 	PrimitiveComponent->SetCollisionEnabled( ECollisionEnabled::QueryAndPhysics );
	// 	PrimitiveComponent->SetGenerateOverlapEvents( true );
	// }
}

AActor* FUtils::DuplicateActor(AActor* Actor)
{
	if (Actor->GetLocalRole() != ROLE_Authority)
	{
		UE_LOG(LogTemp, Error, TEXT("Attempt to duplicate actor without authority."));
		return nullptr;
	}
	FActorSpawnParameters spawnParameters = FActorSpawnParameters();
	spawnParameters.Template = Actor;
	return Actor->GetWorld()->SpawnActor(Actor->GetClass(), &(Actor->GetTransform()), spawnParameters);
}

FString FUtils::BuildConfigPath(const FString& RelPath, bool* bFileExists)
{
	const FString FullPath = FPaths::ProjectConfigDir() + RelPath.TrimChar(TEXT('\\'));
	if ( bFileExists != nullptr )
	{
		*bFileExists = FPlatformFileManager::Get().GetPlatformFile().FileExists(*FullPath);
	}
	return FullPath;
}

const auto AngleToPlusMinus180 = [](float Angle)
{
	Angle = FMath::Fmod(Angle, 360.f);
	if (Angle > 180.f) Angle = 360.f - Angle;
	return Angle;
};

float FUtils::ClampedAngularFInterpTo(const float& Current, const float& Target, const float& DeltaTime,
	const float& InterpSpeed, const float& Range)
{
	return FMath::FInterpTo(AngleToPlusMinus180(Current),
		FMath::Clamp(AngleToPlusMinus180(Target), -Range/2.f, Range/2.f),
		DeltaTime,
		InterpSpeed);
}

const auto RotatorToPlusMinus180Vector = [](const FRotator& Rotator)
{
	return FVector(
		AngleToPlusMinus180(Rotator.Roll),
		AngleToPlusMinus180(Rotator.Pitch),
		AngleToPlusMinus180(Rotator.Yaw));
};

const auto ClampVectorAxis = [](const FVector& Value, const FVector& Min, const FVector& Max)
{
	return FVector(
		FMath::Clamp(Value.X, Min.X, Max.X),
		FMath::Clamp(Value.Y, Min.Y, Max.Y),
		FMath::Clamp(Value.Z, Min.Z, Max.Z));
};

FRotator FUtils::ClampedRInterpTo(const FRotator& Current, const FRotator& Target, const float& DeltaTime,
	const float& InterpSpeed, const FRotator& Range)
{
	const FVector VectorisedHalfRange = FVector(Range.Roll, Range.Pitch, Range.Yaw) / 2.f;
	const FVector VectorisedInterp = FMath::VInterpTo(
		RotatorToPlusMinus180Vector(Current),
		ClampVectorAxis(RotatorToPlusMinus180Vector(Target), -VectorisedHalfRange, VectorisedHalfRange),
		DeltaTime,
		InterpSpeed);
	return FRotator(VectorisedInterp.Y, VectorisedInterp.Z, VectorisedInterp.X);
}

