#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogModularNavigation, Log, All);

class MODULARNAVIGATION_API FModularNavigationUtils
{
public:
	static FVector Component(const FVector& U, const FVector& Direction);
	static float AngleInDegrees(const FVector& U, const FVector& V);
	static FVector ComputeGroundMovement(const FVector& Movement, const FHitResult& Hit, bool bMaintainHorizontalGroundVelocity);
	static void NetSerializeFloat(FArchive& Ar, float& Value, uint32 BitCount, bool& bOutSuccess);

private:
	FModularNavigationUtils() {}
};