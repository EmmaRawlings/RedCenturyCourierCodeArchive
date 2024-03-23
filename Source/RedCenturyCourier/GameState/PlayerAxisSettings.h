// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerAxisSettings.generated.h"

/**
 * Stores custom axis properties, such as sensitivity, acceleration, aim assist, etc.
 */
USTRUCT(BlueprintType)
struct REDCENTURYCOURIER_API FAxisProperties final
{
	GENERATED_BODY()

	friend FArchive& operator<<(FArchive& Ar, FAxisProperties& InProperties)
	{
		Ar << InProperties.Sensitivity;
		Ar << InProperties.TurningBoost;
		Ar << InProperties.TurningBoostRampTime;
		Ar << InProperties.DampeningTime;
		Ar << InProperties.Exponent;
		Ar << InProperties.AimAssistStrength;
		Ar << InProperties.DeadZone;
	
		return Ar;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enhanced Input|User Settings")
	FVector2D Sensitivity = FVector2D(.15f);

	/**
	 * Additional boost, as a multiplier, to apply when stick is pushed to maximum. Set to 0% to disable boost.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enhanced Input|User Settings")
	FVector2D TurningBoost = FVector2D::Zero();

	/**
	 * Time it takes to ramp up to the boost multiplier, when stick is pushed to maximum. Set to 0 to boost immediately.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enhanced Input|User Settings")
	float TurningBoostRampTime = 0.f;

	/**
	 * Time it takes to reach full sensitivity after initial input, to enable small adjustments. Set to 0 to disable dampening.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enhanced Input|User Settings")
	float DampeningTime = 0.f;

	/**
	 * Modify the input curve via exponent. Set to 1 for linear input.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enhanced Input|User Settings")
	float Exponent = 1.f;

	/**
	 * Aim assist to apply. Set to 0 to disable.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enhanced Input|User Settings")
	float AimAssistStrength = 0.f;

	/**
	 * Size of dead-zone, as a ratio.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enhanced Input|User Settings")
	float DeadZone = 0.f;
};

/**
 * 
 */
// UCLASS(BlueprintType/*, Blueprintable*/)
// class REDCENTURYCOURIER_API UPlayerAxisSettings : public UPlayerMappableKeySettings
// {
// 	GENERATED_BODY()
// 	
// public:
// 	/** The category that this player mapping is in */
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
// 	FAxisProperties AxisProperties;
// };
