// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavModeComponent.h"
#include "WallNavModeComponent.generated.h"

/** A UNavigationMode, that represents the action of wall running and/or climbing, can perform a wall jump from this mode using UWallJumpNavigationMode. */
USTRUCT(BlueprintType)
struct FWallNavigationInfo
{
	GENERATED_USTRUCT_BODY()
	
	explicit FWallNavigationInfo(const FVector& InImpactNormal)
	{
		ImpactNormal = InImpactNormal;
		WallInfluence = -FVector::CrossProduct(ImpactNormal, FVector::CrossProduct(ImpactNormal, FVector::UpVector));
		WallInfluence.Normalize();
		bValid = true;
	}
	
	explicit FWallNavigationInfo()
	{
		bValid = false;
	}

	/** Normal of the wall surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Wall Navigation Info")
	FVector ImpactNormal;
	/** A direction, pointing upwards along the wall. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Wall Navigation Info")
	FVector WallInfluence;
	/** Whether this info represents a detected wall that is valid for wall running. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Wall Navigation Info")
	bool bValid;

	bool operator!=(const FWallNavigationInfo& WallNavigationInfo) const
	{
		return ImpactNormal != WallNavigationInfo.ImpactNormal
		|| WallInfluence != WallNavigationInfo.WallInfluence
		|| bValid != WallNavigationInfo.bValid;
	}

	bool operator==(const FWallNavigationInfo& WallNavigationInfo) const
	{
		return ImpactNormal == WallNavigationInfo.ImpactNormal
		&& WallInfluence == WallNavigationInfo.WallInfluence
		&& bValid == WallNavigationInfo.bValid;
	}

	FString GetDebugInfo()
	{
		return FString::Format(TEXT("ImpactNormal = {1}, WallNormal = {2}, WallInfluence = {3}, bValid = {4}"),
			{ImpactNormal.ToString(), WallInfluence.ToString(), bValid});
	}
};

// class MODULARNAVIGATION_API FWallNavModeNetworkPilotData : public FNavModeNetworkPilotData
// {
// public:
// 	FWallNavigationInfo WallNavigationInfo;
//
// 	virtual void Clear() override;
// 	virtual void RecordPreflight(ACharacter* Char, UNavModeComponent* NavMode, float InDeltaTime) override;
// 	virtual void Serialize(FArchive& Ar, UPackageMap* PackageMap) override;
// 	virtual void Fill(ACharacter* Char, UNavModeComponent* NavMode) const override;
// protected:
// 	virtual FNavModeNetworkPilotData* NewInstance() const override;
// 	virtual void CopyFrom(const FNavModeNetworkPilotData* Original) override;
// };

class MODULARNAVIGATION_API FWallNavModeNetworkSavedData : public FNavModeNetworkSavedData
{
public:
	FWallNavigationInfo WallNavigationInfo;
	float TimeSinceJump;
	float Timer;
	FWallNavigationInfo LastUsedWallNavigationInfo;
	float TimeSinceNavEnd;

	virtual void Clear() override;
	virtual void RecordPostUpdate(ACharacter* Char, UNavModeComponent* NavMode, FSavedMove_Character::EPostUpdateMode PostUpdateMode) override;
	virtual void Serialize(FArchive& Ar, UPackageMap* PackageMap) override;
	virtual void Fill(ACharacter* Char, UNavModeComponent* NavMode) const override;
	virtual bool CheckForError(ACharacter* Char, UNavModeComponent* NavMode) const override;
protected:
	virtual FNavModeNetworkSavedData* NewInstance() const override;
	virtual void CopyFrom(const FNavModeNetworkSavedData* Original) override;
};

constexpr float WALL_NAV_END_GRACE_PERIOD = .15f;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MODULARNAVIGATION_API UWallNavModeComponent : public UNavModeComponent
{
	GENERATED_BODY()

	/*friend FWallNavModeNetworkPilotData;*/ friend FWallNavModeNetworkSavedData;

public:
	UWallNavModeComponent();

	// main
	/** How much acceleration to apply in the direction of the wall, to keep the character gripped onto the wall. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Wall Navigation Mode")
	float WallGripAccel;
	/** How long the navigation can be maintained for. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Wall Navigation Mode")
	float Duration;
	/** Whether or not any additional air jumps are reset when navigating. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Wall Navigation Mode")
	bool bResetJumps;
	/** Minimum speed required to be able to enter a wall run/climb. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Wall Navigation Mode")
	float MinimumEntrySpeed;
	/** Minimum incline, in degrees, of the wall required to enter a wall run/climb. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Wall Navigation Mode")
	float MinimumIncline;
	/** When entering a wall run/climb after exiting another wall run/climb, ensure the angle of the new wall compared to the old wall is not less than this threshold.
	 * This is useful to ensure the character cannot repeatedly wall run/climb against the same or similar wall. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"), Category="EsperFPS | Wall Navigation Mode")
	float DisallowedRepeatWallAngleThreshold;
	/** The speed threshold which the wall run must maintain to continue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Wall Navigation Mode")
	float SpeedThreshold;

	// climb
	/** The max speed at which the character may climb up the wall.
	 * This is additionally increased/reduced based on the incline of the wall. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Wall Navigation Mode")
	float ClimbSpeed;
	/** How much acceleration to apply, in order to get up to the climb speed.
	 * This acceleration drops over the course of the navigation, dropping to 0 at the end of the navigation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Wall Navigation Mode")
	float ClimbAccel;
	/** Whether or not to reduce the character's speed to match the climb speed, using the climb acceleration to do so. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Wall Navigation Mode")
	bool bLimitClimbSpeed;
	/** Climbing speed at the end of the wall navigation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Wall Navigation Mode")
	float EndClimbSpeed;
	/** TODO what is this??? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Wall Navigation Mode")
	float MomentumClimbAccelMulti;

	// run
	/** The max speed at which the character may run along the wall. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Wall Navigation Mode")
	float RunSpeed;
	/** How much acceleration to apply, in order to get up to the run speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Wall Navigation Mode")
	float RunAccel;
	/** Whether or not to reduce the character's speed to match the run speed, using the run acceleration to do so. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Wall Navigation Mode")
	bool bLimitRunSpeed;
	
	// wall jump
	/** Velocity change to apply, laterally, in the opposite direction to the wall. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Wall Jump Navigation Mode")
	float WallJumpLateralVelocity;
	/** Initial air control after wall jumping, set low to prevent the character from moving back onto the wall again. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Wall Jump Navigation Mode")
	float WallJumpAirControl;
	/** The time, in seconds, before the air control loss from wall jumping is recovered. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Wall Jump Navigation Mode")
	float WallJumpAirControlRecoveryTime;
	/** Whether or not wall run/climb navigation mode timers are reset when wall jumping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Wall Jump Navigation Mode")
	bool bResetWallNavigationOnWallJump;
	/** TODO doc */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Wall Jump Navigation Mode")
	float WallJumpInputInfluence;
	/** TODO doc */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Wall Jump Navigation Mode")
	float WallJumpMomentumOverride;

	// info
	/** Information on the current detected wall (or lack thereof) for potential wall running. */
	UPROPERTY(EditAnywhere, BlueprintGetter=GetWallNavigationInfo, Category="EsperFPS | Wall Navigation Mode")
	FWallNavigationInfo WallNavigationInfo;

	UFUNCTION(BlueprintPure, BlueprintInternalUseOnly, Category="EsperFPS | Wall Navigation Mode")
	FWallNavigationInfo GetWallNavigationInfo() const
	{
		return WallNavigationInfo;
	}

	UPROPERTY(EditAnywhere, BlueprintGetter=GetTimeSinceJump, Category="EsperFPS | Wall Navigation Mode")
	float TimeSinceJump;

	UFUNCTION(BlueprintPure, BlueprintInternalUseOnly, Category="EsperFPS | Wall Navigation Mode")
	float GetTimeSinceJump() const
	{
		return TimeSinceJump;
	}
	
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnHitCallback(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
					   FVector NormalImpulse, const FHitResult& Hit);

public:
	virtual void Preflight() override;
	virtual bool CanBegin() override;
	virtual void Begin() override;
	virtual void Exec(float DeltaTime) override;
	virtual bool ShouldEnd() override;
	virtual void End() override;
	virtual void Idle(float DeltaTime) override;
	virtual bool DoJump(bool bReplayingMoves) override;
	virtual bool AllowsFirstAirJump() override;
	// virtual FNavModeNetworkPilotData* NewNetworkPilotData() const override
	// {
	// 	return new FWallNavModeNetworkPilotData();
	// }
	virtual FNavModeNetworkSavedData* NewNetworkSavedData() const override
	{
		return new FWallNavModeNetworkSavedData();
	}
	virtual FString GetDebugInfo() override;
private:
	bool AcceptWallNavigationInfoIfValid(const FWallNavigationInfo& Info);
	void AddClimbImpulse(float ImpulseUpwards, bool bVelocityChange) const;
	void ResetTimer();
	void JumpIdle(float DeltaTime);
	float Timer;
	FWallNavigationInfo LastUsedWallNavigationInfo;
	float TimeSinceNavEnd;
	FVector_NetQuantizeNormal HitImpactNormal;
};
 