// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "../GameState/MainGameInstance.h"
#include "../GameState/UserSettings.h"
#include "RedCenturyCourier/GameState/MainInputUserSettings.h"

AMainPlayerController::AMainPlayerController()
{
}

void AMainPlayerController::BindInputActionWithHoldToggle(UInputAction* InputAction, FOnToggleableInputAction Event)
{
	const UMainGameInstance* gameInstance = UMainGameInstance::GetGameInstance(GetWorld());
	checkf(gameInstance, TEXT("MainGameInstance must be configured in: project settings > default game instance."));

	UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	checkf(PlayerEnhancedInputComponent, TEXT("Enhanced input must be configured in project settings (see UE docs)."));

	ToggleStates.Add(InputAction->GetFName(), FToggleableInputState::BuildNew(Event));

	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Ongoing, this, &AMainPlayerController::ToggleableInputOngoing);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Canceled, this, &AMainPlayerController::ToggleableInputCanceled);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &AMainPlayerController::ToggleableInputStarted);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Triggered, this, &AMainPlayerController::ToggleableInputTriggered);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &AMainPlayerController::ToggleableInputCompleted);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::None, this, &AMainPlayerController::ToggleableInputNone);
}

void AMainPlayerController::SetToggle(const UInputAction* InputAction, const bool InState)
{
	checkf(ToggleStates.Contains(InputAction->GetFName()),
		TEXT("Attempt to set toggle state of input which has not been made toggleable (BindInputActionWithHoldToggle)."));
	FToggleableInputState& state = ToggleStates[InputAction->GetFName()];
	
	switch(UUserSettingsManager::GetHoldToggleMethod(InputAction, GetWorld()))
	{
	case Contextual:
		// don't update for hold
		if (state.ActionValue && state.ElapsedSeconds < CONTEXTUAL_HOLD_TOGGLE_THRESHOLD)
		{
			// if we haven't passed the hold/toggle threshold, then we need to make sure toggle doesn't persist
			state.bConfirmedHold = true;
		} else if (!state.bConfirmedHold) {
			state.ActionValue = InState;
		}
		break;
	case Toggle:
		state.ActionValue = InState;
		break;
	case Hold:
	default:
		break;
	}
}

void AMainPlayerController::ToggleableInputOngoing(const FInputActionInstance& InputActionInstance)
{
	ExecuteToggleInput(InputActionInstance.GetSourceAction(), ETriggerEvent::Ongoing);
}

void AMainPlayerController::ToggleableInputCanceled(const FInputActionInstance& InputActionInstance)
{
	ExecuteToggleInput(InputActionInstance.GetSourceAction(), ETriggerEvent::Canceled);
}

void AMainPlayerController::ToggleableInputStarted(const FInputActionInstance& InputActionInstance)
{
	const EHoldToggleMethod HoldToggleMethod = UUserSettingsManager::GetHoldToggleMethod(InputActionInstance.GetSourceAction(), GetWorld());
	FToggleableInputState& state = ToggleStates[InputActionInstance.GetSourceAction()->GetFName()];
	
	switch (HoldToggleMethod)
	{
	case Hold:
		state.ActionValue = true;
		state.ElapsedSeconds = InputActionInstance.GetElapsedTime();
		ExecuteToggleInput(InputActionInstance.GetSourceAction(), ETriggerEvent::Started);
		break;
	case Toggle:
		state.ActionValue = !state.ActionValue;
		if (state.ActionValue) state.ElapsedSeconds = 0.f;
		else state.ElapsedSeconds += GetWorld()->GetRealTimeSeconds() - state.LastRealTimeSeconds;
		ExecuteToggleInput(InputActionInstance.GetSourceAction(), state.ActionValue ? ETriggerEvent::Started : ETriggerEvent::Completed);
		break;
	case Contextual:
		state.bConfirmedHold = false;
		state.ActionValue = !state.ActionValue;
		if (state.ActionValue) state.ElapsedSeconds = 0.f;
		else state.ElapsedSeconds += GetWorld()->GetRealTimeSeconds() - state.LastRealTimeSeconds;
		ExecuteToggleInput(InputActionInstance.GetSourceAction(), state.ActionValue ? ETriggerEvent::Started : ETriggerEvent::Completed);
		break;
	default:
		checkf(false, TEXT("Unrecognised Hold/Toggle method: %e"), HoldToggleMethod);
		break;
	}
	state.LastRealTimeSeconds = GetWorld()->GetRealTimeSeconds();
}

void AMainPlayerController::ToggleableInputTriggered(const FInputActionInstance& InputActionInstance)
{
	FToggleableInputState& state = ToggleStates[InputActionInstance.GetSourceAction()->GetFName()];

	if (state.ActionValue)
	{
		state.ElapsedSeconds += GetWorld()->GetRealTimeSeconds() - state.LastRealTimeSeconds;
		ExecuteToggleInput(InputActionInstance.GetSourceAction(), ETriggerEvent::Triggered);
	}
	
	const EHoldToggleMethod HoldToggleMethod = UUserSettingsManager::GetHoldToggleMethod(InputActionInstance.GetSourceAction(), GetWorld());
	if (HoldToggleMethod == Contextual && state.ElapsedSeconds > CONTEXTUAL_HOLD_TOGGLE_THRESHOLD) state.bConfirmedHold = true;
	
	state.LastRealTimeSeconds = GetWorld()->GetRealTimeSeconds();
}

void AMainPlayerController::ToggleableInputCompleted(const FInputActionInstance& InputActionInstance)
{
	const EHoldToggleMethod HoldToggleMethod = UUserSettingsManager::GetHoldToggleMethod(InputActionInstance.GetSourceAction(), GetWorld());
	FToggleableInputState& state = ToggleStates[InputActionInstance.GetSourceAction()->GetFName()];

	switch (HoldToggleMethod)
	{
	case Hold:
		state.ActionValue = false;
		state.ElapsedSeconds = InputActionInstance.GetElapsedTime();
		ExecuteToggleInput(InputActionInstance.GetSourceAction(), ETriggerEvent::Completed);
		break;
	case Toggle:
		// Toggle doesn't care about button release
		break;
	case Contextual:
		if (state.bConfirmedHold || state.ElapsedSeconds > CONTEXTUAL_HOLD_TOGGLE_THRESHOLD)
		{
			state.bConfirmedHold = false;
			state.ActionValue = false;
			state.ElapsedSeconds = InputActionInstance.GetElapsedTime();
			ExecuteToggleInput(InputActionInstance.GetSourceAction(), ETriggerEvent::Completed);
		}
		break;
	default:
		checkf(false, TEXT("Unrecognised Hold/Toggle method: %e"), HoldToggleMethod);
		break;
	}
	state.LastRealTimeSeconds = GetWorld()->GetRealTimeSeconds();
}

void AMainPlayerController::ToggleableInputNone(const FInputActionInstance& InputActionInstance)
{
	FToggleableInputState& state = ToggleStates[InputActionInstance.GetSourceAction()->GetFName()];

	if (state.ActionValue)
	{
		state.ElapsedSeconds += GetWorld()->GetRealTimeSeconds() - state.LastRealTimeSeconds;
		ExecuteToggleInput(InputActionInstance.GetSourceAction(), ETriggerEvent::Triggered);
	}
	
	state.LastRealTimeSeconds = GetWorld()->GetRealTimeSeconds();
}

void AMainPlayerController::ExecuteToggleInput(const UInputAction* InputAction, const ETriggerEvent& TriggerEvent)
{
	const FToggleableInputState state = ToggleStates[InputAction->GetFName()];
	state.Event.Execute(TriggerEvent, state.ActionValue, state.ElapsedSeconds);
}

void AMainPlayerController::BindBoolWithProps(UInputAction* InputAction, FOnBoolInputAction Event)
{
	const UMainGameInstance* gameInstance = UMainGameInstance::GetGameInstance(GetWorld());
	checkf(gameInstance, TEXT("MainGameInstance must be configured in: project settings > default game instance."));

	UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	checkf(PlayerEnhancedInputComponent, TEXT("Enhanced input must be configured in project settings (see UE docs)."));

	BoolBinds.Add(InputAction->GetFName(), Event);

	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Ongoing, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Canceled, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Triggered, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::None, this, &AMainPlayerController::OnBoundInput);
}

void AMainPlayerController::BindAxis1DWithProps(UInputAction* InputAction, FOnAxis1DInputAction Event)
{
	const UMainGameInstance* gameInstance = UMainGameInstance::GetGameInstance(GetWorld());
	checkf(gameInstance, TEXT("MainGameInstance must be configured in: project settings > default game instance."));

	UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	checkf(PlayerEnhancedInputComponent, TEXT("Enhanced input must be configured in project settings (see UE docs)."));

	Axis1DBinds.Add(InputAction->GetFName(), Event);

	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Ongoing, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Canceled, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Triggered, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::None, this, &AMainPlayerController::OnBoundInput);
}

void AMainPlayerController::BindAxis2DWithProps(UInputAction* InputAction, FOnAxis2DInputAction Event)
{
	const UMainGameInstance* gameInstance = UMainGameInstance::GetGameInstance(GetWorld());
	checkf(gameInstance, TEXT("MainGameInstance must be configured in: project settings > default game instance."));

	UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	checkf(PlayerEnhancedInputComponent, TEXT("Enhanced input must be configured in project settings (see UE docs)."));

	Axis2DBinds.Add(InputAction->GetFName(), Event);

	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Ongoing, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Canceled, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Triggered, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::None, this, &AMainPlayerController::OnBoundInput);
}

void AMainPlayerController::BindAxis3DWithProps(UInputAction* InputAction, FOnAxis3DInputAction Event)
{
	const UMainGameInstance* gameInstance = UMainGameInstance::GetGameInstance(GetWorld());
	checkf(gameInstance, TEXT("MainGameInstance must be configured in: project settings > default game instance."));

	UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	checkf(PlayerEnhancedInputComponent, TEXT("Enhanced input must be configured in project settings (see UE docs)."));

	Axis3DBinds.Add(InputAction->GetFName(), Event);

	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Ongoing, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Canceled, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Triggered, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &AMainPlayerController::OnBoundInput);
	PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::None, this, &AMainPlayerController::OnBoundInput);
}

void AMainPlayerController::OnBoundInput(const FInputActionInstance& InputActionInstance)
{
	const auto InputAction = InputActionInstance.GetSourceAction();
	const auto ActionKey = InputAction->GetFName();
	switch (InputAction->ValueType)
	{
	case EInputActionValueType::Boolean:
		if (BoolBinds.Contains(ActionKey))
			BoolBinds[ActionKey].Execute(InputActionInstance.GetTriggerEvent(), InputActionInstance.GetValue().Get<bool>(),
			InputActionInstance.GetElapsedTime(), InputActionInstance.GetTriggeredTime(), InputAction);
		break;
	case EInputActionValueType::Axis1D:
		if (Axis1DBinds.Contains(ActionKey))
			Axis1DBinds[ActionKey].Execute(InputActionInstance.GetTriggerEvent(), InputActionInstance.GetValue().Get<float>(),
			InputActionInstance.GetElapsedTime(), InputActionInstance.GetTriggeredTime(), InputAction);
		break;
	case EInputActionValueType::Axis2D:
		if (Axis2DBinds.Contains(ActionKey))
		{
			const auto InputSettings = Cast<UMainInputUserSettings>(
				GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()->GetUserSettings());
			FVector2D Value = InputActionInstance.GetValue().Get<FVector2D>();
			const auto MappableSettings = InputAction->GetPlayerMappableKeySettings();
			// TODO is there some core enhanced input service we can inject this stuff into?
			if (MappableSettings != nullptr && InputSettings != nullptr)
			{
				const FAxisProperties Props = InputSettings->GetAxisProperties(MappableSettings->GetMappingName());

				Value = FVector2d(FMath::Pow(Value.X, Props.Exponent), FMath::Pow(Value.Y, Props.Exponent));
				Value = FVector2d(Value.X * Props.Sensitivity.X, Value.Y * Props.Sensitivity.Y);
				// TODO apply other properties
			}
			else
			{
				UE_LOG(LogTemp, Warning,
					TEXT("Attempted to apply Axis properties but no UPlayerMappableKeySettings or UMainInputUserSettings"
					" found. Please set UMainInputUserSettings in the project settings (Engine - Enhanced Input), and"
					" add UPlayerMappableKeySettings to the UInputAction '%s'"), *InputAction->GetName());
			}
			Axis2DBinds[ActionKey].Execute(InputActionInstance.GetTriggerEvent(), Value,
			InputActionInstance.GetElapsedTime(), InputActionInstance.GetTriggeredTime(), InputAction);
		}
		break;
	case EInputActionValueType::Axis3D:
		if (Axis3DBinds.Contains(ActionKey))
			Axis3DBinds[ActionKey].Execute(InputActionInstance.GetTriggerEvent(), InputActionInstance.GetValue().Get<FVector>(),
			InputActionInstance.GetElapsedTime(), InputActionInstance.GetTriggeredTime(), InputAction);
		break;
	default:
		break;
	}
}

