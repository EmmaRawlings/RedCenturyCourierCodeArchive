#include "ModularNavigationUtils.h"

#include "Iris/Serialization/BitPacking.h"

DEFINE_LOG_CATEGORY(LogModularNavigation);

FVector FModularNavigationUtils::Component(const FVector& U, const FVector& Direction)
{
	return FVector::DotProduct(U.GetSafeNormal(), Direction.GetSafeNormal())
		* Direction.GetSafeNormal()
		* U.Length();
}

float FModularNavigationUtils::AngleInDegrees(const FVector& U, const FVector& V)
{
	return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(U, V)));
}

FVector FModularNavigationUtils::ComputeGroundMovement(const FVector& Movement, const FHitResult& Hit, bool bMaintainHorizontalGroundVelocity)
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

void FModularNavigationUtils::NetSerializeFloat(FArchive& Ar, float& Value, uint32 BitCount, bool& bOutSuccess)
{
	if (Ar.IsSaving())
	{
		uint32 QuantizedValue = UE::Net::QuantizeSignedUnitFloat(Value, BitCount);
		bOutSuccess = SerializeOptionalValue<uint32>(true, Ar, QuantizedValue, 0);
	} else {
		uint32 QuantizedValue;
		bOutSuccess = SerializeOptionalValue<uint32>(false, Ar, QuantizedValue, 0);
		Value = UE::Net::DequantizeSignedUnitFloat(QuantizedValue, BitCount);
	}
}