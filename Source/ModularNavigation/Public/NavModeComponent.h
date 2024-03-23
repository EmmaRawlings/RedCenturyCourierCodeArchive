// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavModeComponent.generated.h"

class AModularNavigationCharacter;
class UModularCharacterMovementComponent;
class UModularNavigationCharacter;
class UNavModeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNavMode);
DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FIsAllowedNavMode);

struct MODULARNAVIGATION_API FMovementParameterContext
{
	bool bUseSeparateBrakingFriction;
	float MaxAcceleration;
	float MaxBrakingDeceleration;
	float BrakingFriction;
	float BrakingFrictionFactor;
	float GroundFriction;
	float WalkOntoSteepSlopeVelocityScale;
	bool bAllowResetJumpState;
	float MaxSpeed;
};

UENUM()
enum class ENavModeActive : uint8
{
	Active,
	Inactive,
};

class MODULARNAVIGATION_API FNavModeNetworkPilotData
{
public:
	FNavModeNetworkPilotData(): bIsAllowed(false) {}

	virtual ~FNavModeNetworkPilotData() {}

	bool bIsAllowed;
	
	/** Clear saved move properties, so it can be re-used. */
	virtual void Clear();
	FNavModeNetworkPilotData* Clone() const;
	/** Called to set up this saved move (when initially created) to make a predictive correction. */
	virtual void RecordPreflight(ACharacter* Char, UNavModeComponent* NavMode, float InDeltaTime);
	virtual void Serialize(FArchive& Ar, UPackageMap* PackageMap);
	virtual void Fill(ACharacter* Char, UNavModeComponent* NavMode) const;
	
	/** Returns true if this move is an "important" move that should be sent again if not acked by the server */
	// TODO virtual bool IsImportantMove(const FSavedMove_NavMode& LastAckedMove) const { return false; }
	/** Returns true if this move can be combined with NewMove for replication without changing any behavior */
	// TODO virtual bool CanCombineWith(const FSavedMove_NavMode& NewMove, ACharacter* InCharacter, float MaxDelta) const { return true; }
	/** Combine this move with an older move and update relevant state. */
	// TODO virtual void CombineWith(const FSavedMove_NavMode* OldMove, ACharacter* InCharacter, APlayerController* PC,
	// 	const FVector& OldStartLocation);
protected:
	virtual FNavModeNetworkPilotData* NewInstance() const;
	virtual void CopyFrom(const FNavModeNetworkPilotData* Original);
};

class MODULARNAVIGATION_API FNavModeNetworkSavedData
{
public:
	FNavModeNetworkSavedData() {}
	virtual ~FNavModeNetworkSavedData() {}
	
	/** Clear saved move properties, so it can be re-used. */
	virtual void Clear();
	FNavModeNetworkSavedData* Clone() const;
	/** Set the properties describing the final position, etc. of the moved pawn. */
	virtual void RecordPostUpdate(ACharacter* Char, UNavModeComponent* NavMode, FSavedMove_Character::EPostUpdateMode PostUpdateMode);
	virtual void Serialize(FArchive& Ar, UPackageMap* PackageMap);
	virtual void Fill(ACharacter* Char, UNavModeComponent* NavMode) const;
	virtual bool CheckForError(ACharacter* Char, UNavModeComponent* NavMode) const;

	/** Returns true if this move is an "important" move that should be sent again if not acked by the server */
	// TODO virtual bool IsImportantMove(const FSavedMove_NavMode& LastAckedMove) const { return false; }
	/** Returns true if this move can be combined with NewMove for replication without changing any behavior */
	// TODO virtual bool CanCombineWith(const FSavedMove_NavMode& NewMove, ACharacter* InCharacter, float MaxDelta) const { return true; }
	/** Combine this move with an older move and update relevant state. */
	// TODO virtual void CombineWith(const FSavedMove_NavMode* OldMove, ACharacter* InCharacter, APlayerController* PC,
	// 	const FVector& OldStartLocation);
protected:
	virtual FNavModeNetworkSavedData* NewInstance() const;
	virtual void CopyFrom(const FNavModeNetworkSavedData* Original);
};

/** Abstract class that represents a single nav mode that a character may enter (slide, wall run, wall jump, ledge vault, etc). */
UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MODULARNAVIGATION_API UNavModeComponent : public UActorComponent
{
	GENERATED_BODY()

	friend FNavModeNetworkPilotData; friend FNavModeNetworkSavedData;
public:
	UNavModeComponent();

	// main
	
	/** Check whether or not the mode is currently active.
	 * The mode becomes active when internal conditions are met, the IsAllowedNavMode delegate returns true, and no other mode is active. */
	UFUNCTION(BlueprintPure, Category="Nav Mode")
	bool IsModeActive() const;
	/** Branch, based on whether or not the mode is currently active.
	 * The mode becomes active when internal conditions are met, the IsAllowedNavMode delegate returns true, and no other mode is active.. */
	UFUNCTION(BlueprintCallable, Category="Nav Mode", meta=(ExpandEnumAsExecs=ReturnValue))
	ENavModeActive IfModeActive();
	/** Notifies that the navigation has begun. */
	UPROPERTY(EditAnywhere, BlueprintAssignable, Category="Nav Mode")
	FOnNavMode OnBegin;
	/** Notifies that the navigation has ended. */
	UPROPERTY(EditAnywhere, BlueprintAssignable, Category="Nav Mode")
	FOnNavMode OnEnd;
	/** Asks if the nav mode is allowed to begin, given that internal constraints have been met.
	 * This is normally where you want to handle input-style constraints.
	 * The client will check this, the server will not. */
	UPROPERTY(BlueprintReadWrite, Category="Nav Mode")
	FIsAllowedNavMode IsAllowedNavMode;

protected:
	virtual void BeginPlay() override;
public:
	virtual void Preflight();
	virtual bool IsAllowed() const;
	virtual bool CanBegin();
	virtual void Begin();
	virtual void Exec(float DeltaTime);
	virtual bool ShouldEnd();
	virtual void End();
	virtual void Idle(float DeltaTime);
	/** @return	True if the jump was triggered successfully. */
	virtual bool DoJump(bool bReplayingMoves);
	virtual bool AllowsFirstAirJump();
	virtual bool AllowsAirLedgeStep();
	virtual FNavModeNetworkPilotData* NewNetworkPilotData() const
	{
		return new FNavModeNetworkPilotData();
	}
	virtual FNavModeNetworkSavedData* NewNetworkSavedData() const
	{
		return new FNavModeNetworkSavedData();
	}
	virtual FString GetDebugInfo();
	
protected:
	TObjectPtr<UModularCharacterMovementComponent> GetCharacterMovement() const;
	TObjectPtr<AModularNavigationCharacter> GetCharacter() const;
	FMovementParameterContext NeutralContext() const;
	FMovementParameterContext GetMovementParameterContext() const;
private:
	UPROPERTY()
	TObjectPtr<UModularCharacterMovementComponent> MovementComponent;
	UPROPERTY()
	TObjectPtr<AModularNavigationCharacter> CharacterActor;
	float TimeActive;
	bool bIsAllowed;
};
