// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavModeComponent.h"
#include "WallNavModeComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DashNavModeComponent.generated.h"


class MODULARNAVIGATION_API FDashNavModeNetworkPilotData : public FNavModeNetworkPilotData
{
public:
	FVector_NetQuantizeNormal InputDirection;

	virtual void Clear() override;
	virtual void RecordPreflight(ACharacter* Char, UNavModeComponent* NavMode, float InDeltaTime) override;
	virtual void Serialize(FArchive& Ar, UPackageMap* PackageMap) override;
	virtual void Fill(ACharacter* Char, UNavModeComponent* NavMode) const override;
protected:
	virtual FNavModeNetworkPilotData* NewInstance() const override;
	virtual void CopyFrom(const FNavModeNetworkPilotData* Original) override;
};

class MODULARNAVIGATION_API FDashNavModeNetworkSavedData : public FNavModeNetworkSavedData
{
public:
	float Timer;
	float CooldownTimer;
	FVector_NetQuantizeNormal Direction;

	virtual void Clear() override;
	virtual void RecordPostUpdate(ACharacter* Char, UNavModeComponent* NavMode, FSavedMove_Character::EPostUpdateMode PostUpdateMode) override;
	virtual void Serialize(FArchive& Ar, UPackageMap* PackageMap) override;
	virtual void Fill(ACharacter* Char, UNavModeComponent* NavMode) const override;
	virtual bool CheckForError(ACharacter* Char, UNavModeComponent* NavMode) const override;
protected:
	virtual FNavModeNetworkSavedData* NewInstance() const override;
	virtual void CopyFrom(const FNavModeNetworkSavedData* Original) override;
};

/** Abstract class, representing a means of controlling when a character may air dash. */
UCLASS(BlueprintType, Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class UAirDashNavController : public UActorComponent
{
	GENERATED_BODY()
public:
	virtual void Begin(ACharacter* Character)
	{
		unimplemented();
	}
	virtual bool HasAirDashes(ACharacter* Character)
	{
		unimplemented();
		return false;
	}

	virtual void Idle(ACharacter* Character)
	{
		unimplemented();
	}
};

/** A UAirDashController, that ties air dashing and air jumping together, meaning that both will consume the same resource. */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UAirDashTiedToAirJumpsNavController : public UAirDashNavController
{
	GENERATED_BODY()
public:
	UAirDashTiedToAirJumpsNavController() {}
	virtual void Begin(ACharacter* Character) override
	{
		Character->ResetJumpState();
		if (Character->JumpCurrentCount == 0) Character->JumpCurrentCount++;
		Character->JumpCurrentCount++;
		Character->JumpCurrentCountPreJump = Character->JumpCurrentCount;
	}
	virtual bool HasAirDashes(ACharacter* Character) override
	{
		return Character->JumpCurrentCount < Character->JumpMaxCount;
	}
	virtual void Idle(ACharacter* Character) override
	{
		// do nothing
	}
};

/** A UAirDashController, that defines a static number of air dashes the character may perform. */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UStaticAirDashLimitNavController : public UAirDashNavController
{
	GENERATED_BODY()
public:
	UStaticAirDashLimitNavController()
	{
		AirDashes = 1;
		bResetAirDashesOnWallJump = true;
		AirDashesRemaining = 0;
	}
	virtual void BeginPlay() override
	{
		AirDashesRemaining = AirDashes;
	}
	virtual void Begin(ACharacter* Character) override
	{
		AirDashesRemaining--;
	}
	virtual bool HasAirDashes(ACharacter* Character) override
	{
		return AirDashesRemaining > 0;
	}
	virtual void Idle(ACharacter* Character) override
	{
		if (AirDashesRemaining < AirDashes)
		{
			if (Character->GetCharacterMovement()->IsMovingOnGround())
				AirDashesRemaining = AirDashes;
			else if (bResetAirDashesOnWallJump)
			{
				TArray<UWallNavModeComponent*> WallNavModes;
				Character->GetComponents<UWallNavModeComponent*>(WallNavModes);
	
				for (const UWallNavModeComponent* WallNavMode : WallNavModes)
				{
					if (WallNavMode->IsModeActive())
					{
						AirDashesRemaining = AirDashes;
						break;
					}
				}
			}
		}
	}
	/** The maximum number of allowed air dashes per reset (resets on ground). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 AirDashes;
	/** Whether or not to also reset the number of allowed air dashes after a wall jump. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bResetAirDashesOnWallJump;
private:
	uint8 AirDashesRemaining;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MODULARNAVIGATION_API UDashNavModeComponent : public UNavModeComponent
{
	GENERATED_BODY()

	friend FDashNavModeNetworkPilotData; friend FDashNavModeNetworkSavedData;
	
public:
	UDashNavModeComponent();

	// main
	/** The distance traveled by the dash. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash Nav Mode")
	float Distance;
	/** The duration of the dash. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash Nav Mode")
	float Duration;
	/** The distance traveled by the dash, in air, compared to the usual grounded dash. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash Nav Mode")
	float AirDistanceRatio;
	/** The duration of the dash, in air, compared to the usual grounded dash. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash Nav Mode")
	float AirDurationRatio;
	/** Whether the dash is limited to lateral motion only (non-vertical). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash Nav Mode")
	bool bLateralOnly;
	/** Proportion of momentum to maintain after the dash ends.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash Nav Mode")
	float MaxInheritFactor;
	/** How much air control the character must have before they are allowed to use the air dash. This can be useful for
	 * preventing players from repeatedly wall running/climbing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash Nav Mode")
	float MinimumAirControlForAirDash;
	/** Whether or not the air dash distance/speed should be proportional to the character's air control. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash Nav Mode")
	bool bAirControlAffectsAirDashDistance;
	/** Which air dash method to use, each controller has its own set of parameters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash Nav Mode")
	TObjectPtr<UAirDashNavController> AirDashController;
	/** Time, in seconds, before the dash may be used again (after the last one has ended). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash Nav Mode")
	float Cooldown;

	/** Use travel curve for ground dashes (see TravelCurve). */
	UPROPERTY(EditAnywhere, Category="Dash Nav Mode")
	bool bUseTravelCurveOnGround;
	/** Use travel curve for air dashes (see TravelCurve). */
	UPROPERTY(EditAnywhere, Category="Dash Nav Mode")
	bool bUseTravelCurveInAir;
	/** Rather than performing a dash with linear constant velocity, use this property to curve the velocity of the dash (e.g. dash is faster at the beginning but slower towards the end). */
	UPROPERTY(EditAnywhere, Category="Dash Nav Mode")
	UCurveFloat* TravelCurve;

	UFUNCTION(BlueprintPure, BlueprintInternalUseOnly, Category="Dash Nav Mode")
	FVector_NetQuantizeNormal GetDirection() const
	{
		return Direction;
	}
	
	virtual void BeginPlay() override;
	
	virtual void Preflight() override;
	virtual bool CanBegin() override;
	virtual void Begin() override;
	virtual void Exec(float DeltaTime) override;
	virtual bool ShouldEnd() override;
	virtual void End() override;
	virtual void Idle(float DeltaTime) override;
	virtual FNavModeNetworkPilotData* NewNetworkPilotData() const override
	{
		return new FDashNavModeNetworkPilotData();
	}
	virtual FNavModeNetworkSavedData* NewNetworkSavedData() const override
	{
		return new FDashNavModeNetworkSavedData();
	}
	virtual FString GetDebugInfo() override;
private:
	float Timer;
	float CooldownTimer;
	FVector_NetQuantizeNormal InputDirection;
	UPROPERTY(BlueprintGetter=GetDirection, Category="Dash Nav Mode")
	FVector_NetQuantizeNormal Direction;
	
	FTimeline TravelTimeline;
	UFUNCTION()
	void HandleTravelProgress(float Value);
	float LastTravelTimelineValue;
	float LastTravelTimelineTime;
	bool bInAirDash;
	
	friend class UAirDashNavController;
};
