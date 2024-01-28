// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "../Utility/Delegates.h"
#include "InputAction.h"
#include "MainPlayerController.generated.h"

constexpr float CONTEXTUAL_HOLD_TOGGLE_THRESHOLD = .15f;

USTRUCT(BlueprintType)
struct FToggleableInputState
{
	GENERATED_BODY()

	FOnToggleableInputAction Event;
	bool ActionValue;
	float ElapsedSeconds;
	double LastRealTimeSeconds;
	bool bConfirmedHold;

	static FToggleableInputState BuildNew(const FOnToggleableInputAction& Event) {
		FToggleableInputState Result;
		Result.Event = Event;
		Result.ActionValue = false;
		Result.ElapsedSeconds = 0.f;
		Result.LastRealTimeSeconds = 0.;
		Result.bConfirmedHold = false;
		return Result;
	}
};

UCLASS()
class REDCENTURYCOURIER_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMainPlayerController();
	/** Bind event to input, with support for Hold/Toggle behaviour. */
	UFUNCTION(BlueprintCallable, Category = "Main Player Controller")
	void BindInputActionWithHoldToggle(UInputAction* InputAction, FOnToggleableInputAction Event);
	/** Set toggle state of the input action. */
	UFUNCTION(BlueprintCallable, Category = "Main Player Controller")
	void SetToggle(const UInputAction* InputAction, const bool InState);
protected:
	TMap<FName, FToggleableInputState> ToggleStates;
	void ToggleableInputOngoing(const FInputActionInstance& InputActionInstance);
	void ToggleableInputCanceled(const FInputActionInstance& InputActionInstance);
	void ToggleableInputStarted(const FInputActionInstance& InputActionInstance);
	void ToggleableInputTriggered(const FInputActionInstance& InputActionInstance);
	void ToggleableInputCompleted(const FInputActionInstance& InputActionInstance);
	void ToggleableInputNone(const FInputActionInstance& InputActionInstance);
	void ExecuteToggleInput(const UInputAction* InputAction, const ETriggerEvent& TriggerEvent);
};
