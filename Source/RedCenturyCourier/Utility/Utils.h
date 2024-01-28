#pragma once

#include "CoreMinimal.h"
#include "Utils.generated.h"

// DOCSONLY: namespace Utility {
class REDCENTURYCOURIER_API FUtils final
{
public:
	static FRotator RandomSpread(FRotator Direction, float Spread);
	static float AngleInDegrees(const FVector& U, const FVector& V);
	static FVector Component(const FVector& U, const FVector& Direction);
	// static FVector ProjectToPlane(FVector U, FVector Normal, bool IncreaseOnly);
	static FVector Apex(const FVector& Velocity, const FVector& Acceleration);
	static float VelocityToReachApex(float Apex, float Acceleration);
	static float LimitImpulse(float SpeedChange, float Speed, float MaxSpeed);
	static float Adjustment(float Current, float AdjustTo, float DeltaTime, float AdjustmentSpeed);
	static FVector ComputeGroundMovement(FVector Movement, const FHitResult& Hit, bool bMaintainHorizontalGroundVelocity);
	static void SetActorActive(AActor* Actor, bool inActive);
	static AActor* DuplicateActor(AActor* Actor);
	static FString BuildConfigPath(const FString& RelPath, bool* bFileExists = nullptr);
	static float ClampedAngularFInterpTo(const float& Current, const float& Target, const float& DeltaTime,
		const float& InterpSpeed, const float& Range);
	static FRotator ClampedRInterpTo(const FRotator& Current, const FRotator& Target, const float& DeltaTime,
		const float& InterpSpeed, const FRotator& Range);
private:
	FUtils() {}
};

/** A set of utility functions. */
UCLASS()
class REDCENTURYCOURIER_API UUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/** @return the angle, in degrees, between two vectors. */
	UFUNCTION(BlueprintPure, Category="Utils Library")
	static float AngleInDegrees(const FVector& U, const FVector& V)
	{       
		return FUtils::AngleInDegrees(U, V);
	}
	
	/** @return the component of the vector, in the direction of the other vector. */
	UFUNCTION(BlueprintPure, Category="Utils Library")
	static FVector Component(const FVector& U, const FVector& Direction)
	{
		return FUtils::Component(U, Direction);
	}
	
	/** @return calculate the projected apex, given the current velocity, and the constant acceleration */
	UFUNCTION(BlueprintCallable, Category="Utils Library")
	static FVector Apex(const FVector& Velocity, const FVector& Acceleration)
	{
		return FUtils::Apex(Velocity, Acceleration);
	}
	
	/** @return Speed + SpeedChange, limited by MaxSpeed */
	UFUNCTION(BlueprintCallable, Category="Utils Library")
	static float LimitImpulse(float SpeedChange, float Speed, float MaxSpeed)
	{
		return FUtils::LimitImpulse(SpeedChange, Speed, MaxSpeed);
	}
	
	
	/** @return the Current value, adjusted in the direction of AdjustTo, at the AdjustmentSpeed, over time */
	UFUNCTION(BlueprintCallable, Category="Utils Library")
	static float Adjustment(float Current, float AdjustTo, float DeltaTime, float AdjustmentSpeed)
	{
		return FUtils::Adjustment(Current, AdjustTo, DeltaTime, AdjustmentSpeed);
	}
 
	/** Sets the actor to be active or inactive. 
	 * Controls:
	 *  - In game visibility of the actor
	 *  - Whether the actor will 'tick' each frame
	 *  - Whether collisions are enabled for the actor
	 *  - Whether the child components are active
	 *
	 *  TODO some actors may need to be able to define their own activate/deactivate logic
	 *  (not all actors need to tick, for example)
	 */
	UFUNCTION(BlueprintCallable, Category="Utils Library")
	static void SetActorActive(AActor* Actor, bool inActive)
	{       
		return FUtils::SetActorActive(Actor, inActive);
	}
 
	/** @return a duplicate of the specified actor. */
	UFUNCTION(BlueprintCallable, Category="Utils Library")
	static AActor* DuplicateActor(AActor* Actor)
	{       
		return FUtils::DuplicateActor(Actor);
	}

	/** Float interpolation, that is clamped to the given range, where the input and result are treated as an angle/bearing. */
	UFUNCTION(BlueprintPure, Category="Utils Library")
	static float ClampedAngularFInterpTo(const float Current, const float Target, const float DeltaTime,
		const float InterpSpeed, const float Range)
	{       
		return FUtils::ClampedAngularFInterpTo(Current, Target, DeltaTime, InterpSpeed, Range);
	}
	
	/** Rotator interpolation, that is clamped to the given range. */
	UFUNCTION(BlueprintPure, Category="Utils Library")
	static FRotator ClampedRInterpTo(const FRotator Current, const FRotator Target, const float DeltaTime,
		const float InterpSpeed, const FRotator Range)
	{       
		return FUtils::ClampedRInterpTo(Current, Target, DeltaTime, InterpSpeed, Range);
	}
};
// DOCSONLY: }
