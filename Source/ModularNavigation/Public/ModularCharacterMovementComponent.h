// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularCharacterMovementReplication.h"
#include "NavModeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ModularCharacterMovementComponent.generated.h"

class UNavModeComponent;

class MODULARNAVIGATION_API FSavedModularMove_Character : public FSavedMove_Character
{
public:
	// Need StartNavModeKey to verify if the move should be flushed immediately, see post update, not serialized
	uint8 StartNavModeKey;
	TMap<uint8, FNavModeNetworkPilotData*> NavModePilotData;
	uint8 SavedNavModeKey;
	TMap<uint8, FNavModeNetworkSavedData*> NavModeSavedData;
	virtual void Clear() override;
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;
	virtual void PostUpdate(ACharacter* C, EPostUpdateMode PostUpdateMode) override;
	virtual bool IsImportantMove(const FSavedMovePtr& LastAckedMove) const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
};

class MODULARNAVIGATION_API FNetworkPredictionData_Client_Character_ModularMovement : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Client_Character_ModularMovement(const UCharacterMovementComponent& ClientMovement):
		FNetworkPredictionData_Client_Character(ClientMovement)
	{
	}
	virtual FSavedMovePtr AllocateNewMove() override;
};

class MODULARNAVIGATION_API FNetworkPredictionData_Server_ModularCharacter : public FNetworkPredictionData_Server_Character
{
public:
	FNetworkPredictionData_Server_ModularCharacter(const UCharacterMovementComponent& ServerMovement)
		: FNetworkPredictionData_Server_Character(ServerMovement), PendingAdjustmentNavModeKey(0)
	{
	}

	uint8 PendingAdjustmentNavModeKey;
	TMap<uint8, FNavModeNetworkSavedData*> PendingAdjustmentNavModeData;
};

struct FAirControlDampenInfo
{
	float RecoveryPerSecond;
	float NormalAirControl;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MODULARNAVIGATION_API UModularCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	friend UNavModeComponent; friend FSavedModularMove_Character; friend FCharacterNetworkModularMoveData; friend FCharacterModularMoveResponseDataContainer;
public:
	// Saved Move
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual class FNetworkPredictionData_Server* GetPredictionData_Server() const override;

	UModularCharacterMovementComponent();

	/** Whether or not to allow the attached character to step onto steep slopes whilst walking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modular Character Movement")
	bool bAllowWalkOntoSteepSlope;
	/** How much momentum/velocity to carry over, when stepping onto steep slopes whilst walking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = 0, UIMin = 0), Category="Modular Character Movement")
	float WalkOntoSteepSlopeVelocityScale;
	/** The maximum difference in incline that the character may step onto, when stepping onto steep slopes whilst walking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = 0, UIMin = 0), Category="Modular Character Movement")
	float WalkOntoSteepSlopeMaxAngleDelta;

	/** Toggle execution of nav mode components. Usually should be left on but turning off nav mode execution can
	* help when diagnosing issues with the movement system. */
	UPROPERTY(EditAnywhere, Category="Modular Character Movement")
	bool bExecNavModes;

	/** Enable smooth crouching, instead of instantly entering/exiting crouch state. */
	UPROPERTY(EditAnywhere, Category="Modular Character Movement")
	bool bUseSmoothCrouch;
	/** The curve that dictate progress towards crouching and uncrouching when using smooth crouch. */
	UPROPERTY(EditAnywhere, Category="Modular Character Movement")
	UCurveFloat* CrouchCurve;
	/** TODO emling doc this... */
	UPROPERTY(EditAnywhere, Category="Modular Character Movement")
	float SmoothCrouchedStateThreshold;
	/** TODO emling doc this... */
	UPROPERTY(EditAnywhere, Category="Modular Character Movement")
	float SmoothUnCrouchedStateThreshold;
	
	/** The highest point, relative to the character's feet, where a ledge is checked to step.
	*	Recommended this be set to sit around the character's knees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modular Character Movement")
	float InAirStepMaxHeight;
	/** How far to scan for a ledge to step up. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modular Character Movement")
	float InAirStepMaxDist;
	/** Beyond this limit, the character will not perform a ledge step. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modular Character Movement")
	float InAirStepMaxVerticalSpeed;
	/** Whether to set the vertical speed to match the ledge when performing a ledge step. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modular Character Movement")
	bool bInAirStepResetVerticalSpeed;

	virtual void BeginPlay() override;
protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	// Nav Modes
	void MapOutNavModesOnBeginPlay();
public:
	virtual void SwitchNavMode(UNavModeComponent* NavModeComponent);
	virtual bool IsAnyNavModeActive();
	virtual bool IsNavModeActive(const UNavModeComponent* NavModeComponent);
	virtual bool DoJump(bool bReplayingMoves) override;
	virtual bool NavModeAllowsFirstAirJump();

	// Smooth Crouch
	virtual void Crouch(bool bClientSimulation = false) override;
	virtual void UnCrouch(bool bClientSimulation = false) override;
private:
	// UFUNCTION()
	void ProcessSmoothCrouch(float DeltaTime);
	bool HandleCrouchProgress(float Value);

	// Movement Parameter Context
public:
	virtual void UseMovementParameterContext(FMovementParameterContext InMovementParameterContext);
	virtual void UnsetMovementParameterContext();
protected:
	FMovementParameterContext NeutralContext() const;
	FMovementParameterContext GetMovementParameterContext() const;
public:
	virtual float GetMaxSpeed() const override;
	virtual float GetWalkOntoSteepSlopeVelocityScale() const;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	virtual void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration) override;
	virtual void PhysWalking(float deltaTime, int32 Iterations) override;
	virtual void PhysNavWalking(float deltaTime, int32 Iterations) override;

	// Dampen Air Control
	// TODO
	// virtual float GetCurrentAirControl();
	// virtual FVector GetFallingLateralAcceleration(float DeltaTime) override;
	virtual void DampenAirControl(float DampenedAirControl, float RecoveryTime);
	virtual void ResetAirControlDampening();
	virtual void StartNewPhysics(float deltaTime, int32 Iterations) override;

	// Allow Walk Onto Steep Slope
protected:
	virtual float SlideAlongSurface(const FVector& Delta, float Time, const FVector& Normal, FHitResult& Hit, bool bHandleImpact) override;

	// In Air Step
	virtual void PhysFalling(float deltaTime, int32 Iterations) override;

	// Jump Whilst Crouched
public:
	virtual bool CanAttemptJump() const override;

protected:
	virtual void ControlledCharacterMove(const FVector& InputVector, float DeltaSeconds) override;
	virtual void MoveAutonomous( float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel) override;
	virtual void PerformMovement(float DeltaTime) override;
	virtual bool ServerCheckClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) override;
	virtual void ServerMoveHandleClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) override;
	virtual bool ClientUpdatePositionAfterServerUpdate() override;
	virtual void ClientAdjustPosition_Implementation(float TimeStamp, FVector NewLoc, FVector NewVel, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode, TOptional<FRotator> OptionalRotation = TOptional<FRotator>()) override;

private:
	FVector* LastSurfaceNormal;
	UPROPERTY()
	TMap<uint8, UNavModeComponent*> NavModeByKey;
	UPROPERTY()
	TMap<UNavModeComponent*, uint8> KeyByNavMode;
	uint8 NavModeKey;
	FCharacterNetworkModularMoveDataContainer NetworkMoveDataContainer;
	FCharacterModularMoveResponseDataContainer MoveResponseDataContainer;
	bool bMovementParameterOverride;
	FMovementParameterContext MovementParameterContext;
	float SmoothCrouchTime;
	float CrouchAlpha;
	float UnCrouchedHalfHeight;
	// TODO smooth crouch replication bool bCrouchIsClientSimulation;
	bool bCrouchIntent;
	// FTimeline CrouchTimeline;
	FAirControlDampenInfo AirControlDampenInfo;
};
