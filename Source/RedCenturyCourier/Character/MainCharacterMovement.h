// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularCharacterMovementComponent.h"
#include "MainCharacterMovement.generated.h"

/** Augments UE's character movement component, with additional functionality: 'Rush' (builds as the character maintains forward momentum). */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class REDCENTURYCOURIER_API UMainCharacterMovement : public UModularCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UMainCharacterMovement();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Rush")
	float MaxRush;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Rush")
	float PassiveRushGain;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Rush")
	float RushTimeout;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Rush")
	float RushTimeoutSpeedTolerance;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Rush")
	float PassiveRushLoss;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Rush")
	float Rush;
	UPROPERTY(EditAnywhere, Category="Rush")
	UCurveFloat* RushSpeedMultiCurve;
	UPROPERTY(EditAnywhere, Category="Rush")
	UCurveFloat* RushTimeMultiCurve;
protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	// virtual bool MoveUpdatedComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* OutHit = NULL, ETeleportType Teleport = ETeleportType::None) override;
	float RushTimeoutTimer;
public:
	virtual float GetMaxSpeed() const override;
	virtual float GetRushTimeDilation();

};

// DOCSONLY: }
