// Fill out your copyright notice in the Description page of Project Settings.

#include "UserSettings.h"

#include "JsonObjectConverter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedActionKeyMapping.h"
#include "MainGameInstance.h"
#include "GameplayTagContainer.h"
#include "../Utility/Utils.h"
#include "UserSettings/EnhancedInputUserSettings.h"

DEFINE_LOG_CATEGORY(LogUserSettings);

UUserSettingsManager::UUserSettingsManager()
{
	FilePathRel = TEXT("Settings.json");
	Settings.VideoSettings.FieldOfView = 110.f;
	Settings.AudioSettings.AudioChannels.Push(FAudioChannel::BuildNew(TEXT("Master"), .3f));
	Settings.AudioSettings.AudioChannels.Push(FAudioChannel::BuildNew(TEXT("Sound Effects"), 1.f));
	Settings.AudioSettings.AudioChannels.Push(FAudioChannel::BuildNew(TEXT("Dialogue"), 1.f));
	Settings.AudioSettings.AudioChannels.Push(FAudioChannel::BuildNew(TEXT("Music"), 1.f));
	Settings.AudioSettings.AudioChannels.Push(FAudioChannel::BuildNew(TEXT("Ambient"), 1.f));
	Settings.ControlsSettings.SensitivityX = 5.f;
	Settings.ControlsSettings.SensitivityY = 5.f;
	Settings.ControlsSettings.bHoldJumpToWallRunClimb = false;
	Settings.ControlsSettings.ToggleableInputs.Add(FName(TEXT("Sprint")), FToggleableInput::BuildNew(Contextual));
	Settings.ControlsSettings.ToggleableInputs.Add(FName(TEXT("Crouch")), FToggleableInput::BuildNew(Contextual));
	Settings.ControlsSettings.ToggleableInputs.Add(FName(TEXT("Aim")), FToggleableInput::BuildNew(Contextual));
	Settings.AccessibilitySettings.ScreenShakeIntensity = .5f;
	Settings.AccessibilitySettings.ScreenTiltStrength = .5f;
	Settings.AccessibilitySettings.CameraMotionStrength = .5f;
	Settings.AccessibilitySettings.IteractableHighlight = FColor::Transparent;
	Settings.AccessibilitySettings.EnemyHighlight = FColor::Transparent;
	Settings.AccessibilitySettings.AllyHighlight = FColor::Transparent;
}

void UUserSettingsManager::Init()
{
	// TODO epic added a user settings object, use that instead?
	// if (GetWorld() && GetWorld()->GetFirstPlayerController() && GetWorld()->GetFirstPlayerController()->GetLocalPlayer())
	// {
	// 	checkf(GetWorld()->GetFirstPlayerController()->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(),
	// 		TEXT("UEnhancedInputLocalPlayerSubsystem not found, ensure you have Enhanced Input enabled."));
	// 	// load existing settings file, or save defaults if there is no file
	// 	if (!Load())
	// 	{
	// 		UE_LOG(LogUserSettings, Warning, TEXT("Creating and saving default user settings file (no prior settings file was found)."));
	// 		if (!Save())
	// 			UE_LOG(LogUserSettings, Warning, TEXT("Unable to save new user settings file on start up."));
	// 	}
	// }
	// else
	// {
	// 	// TODO can we schedule for next frame? Or is there a world load delegate we can use?
	// 	FTimerHandle TimerHandle;
	// 	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UUserSettingsManager::Init, 0.1f);
	// }
}

bool UUserSettingsManager::Load()
{
	if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
	{
		UE_LOG(LogUserSettings, Warning, TEXT("World player controller not found."));
		return false;
	}
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstPlayerController()->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogUserSettings, Warning, TEXT("ULocalPlayer not found."));
		return false;
	}
	const UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSystem)
	{
		UE_LOG(LogUserSettings, Warning, TEXT("UEnhancedInputLocalPlayerSubsystem not found."));
		return false;
	}
	UEnhancedInputUserSettings* InputUserSettings = InputSystem->GetUserSettings();
	if (!InputUserSettings)
	{
		UE_LOG(LogUserSettings, Warning, TEXT("UEnhancedInputUserSettings not found."));
		return false;
	}
	
	bool bFileExists;
	const FString FullPath = FUtils::BuildConfigPath(FilePathRel, &bFileExists);
	if (!bFileExists)
	{
		UE_LOG(LogUserSettings, Warning, TEXT("Settings file not found at: %s"), *FullPath);
		return false;
	}

	FString JsonData;
	if (!FFileHelper::LoadFileToString(JsonData, *FullPath))
	{
		UE_LOG(LogUserSettings, Warning, TEXT("Settings file could not be loaded into memory, file path: %s"), *FullPath);
		return false;
	}

	if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonData, &Settings, 0, 0))
	{
		UE_LOG(LogUserSettings, Warning, TEXT("Failed to convert settings file to settings struct, file path: %s"), *FullPath);
		return false;
	}

	TArray<FName> MappingNames;
	Settings.ControlsSettings.Bindings.GetKeys(MappingNames);
	FGameplayTagContainer FailureReason;
	InputUserSettings->ResetKeyProfileToDefault(InputUserSettings->GetCurrentKeyProfileIdentifier(), FailureReason);
	if (!FailureReason.IsEmpty())
	{
		UE_LOG(LogUserSettings, Warning, TEXT("Input mapping failed! Reasoning: %s"), *FailureReason.ToString());
		return false;
	}
	for (const FName MappingName : MappingNames)
	{
		const FInputBindingKeys inputBindingKeys = Settings.ControlsSettings.Bindings[MappingName];
		const TArray<FKey> MappingKeys = inputBindingKeys.Keys;
		for (int32 i = 0; i < MappingKeys.Num(); i++)
		{
			FMapPlayerKeyArgs Args = {};
			Args.MappingName = MappingName;
			Args.NewKey = MappingKeys[i];
			Args.Slot = static_cast<EPlayerMappableKeySlot>(i);

			InputUserSettings->MapPlayerKey(Args, FailureReason);

			if (!FailureReason.IsEmpty())
			{
				UE_LOG(LogUserSettings, Warning, TEXT("Input mapping failed! Reasoning: %s"), *FailureReason.ToString());
				return false;
			}
		}
	}
	// TODO we're just using this field for save/load, not used in game... consider improving the structure for this somehow
	Settings.ControlsSettings.Bindings = TMap<FName, FInputBindingKeys>();

	OnUserSettingsLoaded.Broadcast(Settings);
	return true;
}

bool UUserSettingsManager::Save()
{
	if (!GetWorld())
	{
		return false;
	}
	if (!GetWorld()->GetFirstPlayerController())
	{
		return false;
	}
	
	if (const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstPlayerController()->GetLocalPlayer())
	{
		if (const UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (const UEnhancedInputUserSettings* InputUserSettings = InputSystem->GetUserSettings())
			{
				TMap<FName, FInputBindingKeys> bindings = TMap<FName, FInputBindingKeys>();
				for (const FEnhancedActionKeyMapping playerMapping : InputSystem->GetAllPlayerMappableActionKeyMappings())
				{
					const FName MappingName = playerMapping.GetMappingName();
					TArray<FKey> PlayerMappedKeys;
					if (const UEnhancedPlayerMappableKeyProfile* KeyProfile = InputUserSettings->GetCurrentKeyProfile())
					{
						FPlayerMappableKeyQueryOptions Opts = {};
						Opts.MappingName = MappingName;
						KeyProfile->QueryPlayerMappedKeys(Opts, OUT PlayerMappedKeys);
					}
					
					bindings.Add(MappingName, FInputBindingKeys::BuildNew(PlayerMappedKeys));
				}
				// TODO the json end up being a bit overly verbose, consider adding some custom json parsing for Bindings
				Settings.ControlsSettings.Bindings = bindings;
			} else return false;
		} else return false;
	} else return false;
	
	const FString FullPath = FUtils::BuildConfigPath(FilePathRel);
	FString JsonData;
	if (!FJsonObjectConverter::UStructToJsonObjectString(Settings, JsonData))
	{
		UE_LOG(LogUserSettings, Warning, TEXT("Failed to convert settings struct to settings file, file path: %s"), *FullPath);
		// TODO we're just using this field for save/load, not used in game... consider improving the structure for this somehow
		Settings.ControlsSettings.Bindings = TMap<FName, FInputBindingKeys>();
		return false;
	}
	// TODO we're just using this field for save/load, not used in game... consider improving the structure for this somehow
	Settings.ControlsSettings.Bindings = TMap<FName, FInputBindingKeys>();
	
	if (!FFileHelper::SaveStringToFile(JsonData, *FullPath))
	{
		UE_LOG(LogUserSettings, Warning, TEXT("Could not save settings to file, file path: %s"), *FullPath);
		return false;
	}
	
	return true;
}

void UUserSettingsManager::Apply()
{
}

FVideoUserSettings UUserSettingsManager::GetVideoSettings() const
{
	return Settings.VideoSettings;
}

FAudioUserSettings UUserSettingsManager::GetAudioSettings() const
{
	return Settings.AudioSettings;
}

FControlsUserSettings UUserSettingsManager::GetControlsSettings() const
{
	return Settings.ControlsSettings;
}

FAccessibilityUserSettings UUserSettingsManager::GetAccessibilitySettings() const
{
	return Settings.AccessibilitySettings;
}

FVideoUserSettings UUserSettingsManager::SetVideoSettings(const FVideoUserSettings& NewSettings)
{
	Settings.VideoSettings = NewSettings;
	return Settings.VideoSettings;
}

FAudioUserSettings UUserSettingsManager::SetAudioSettings(const FAudioUserSettings& NewSettings)
{
	Settings.AudioSettings = NewSettings;
	return Settings.AudioSettings;
}

FControlsUserSettings UUserSettingsManager::SetControlsSettings(const FControlsUserSettings& NewSettings)
{
	Settings.ControlsSettings = NewSettings;
	return Settings.ControlsSettings;
}

FAccessibilityUserSettings UUserSettingsManager::SetAccessibilitySettings(const FAccessibilityUserSettings& NewSettings)
{
	Settings.AccessibilitySettings = NewSettings;
	return Settings.AccessibilitySettings;
}

UUserSettingsManager* UUserSettingsManager::GetUserSettingsManager(const UObject* WorldContextObject)
{
	const UMainGameInstance* gameInstance = UMainGameInstance::GetGameInstance(WorldContextObject);
	if (gameInstance != nullptr)
	{
		return gameInstance->UserSettingsManager;
	}
	return nullptr;
}

FVideoUserSettings UUserSettingsManager::GetUserVideoSettings(const UObject* WorldContextObject)
{
	const UUserSettingsManager* userSettingsManager = GetUserSettingsManager(WorldContextObject);
	if (!userSettingsManager) return FVideoUserSettings();
	return userSettingsManager->GetVideoSettings();
}

FAudioUserSettings UUserSettingsManager::GetUserAudioSettings(const UObject* WorldContextObject)
{
	const UUserSettingsManager* userSettingsManager = GetUserSettingsManager(WorldContextObject);
	if (!userSettingsManager) return FAudioUserSettings();
	return userSettingsManager->GetAudioSettings();
}

FControlsUserSettings UUserSettingsManager::GetUserControlsSettings(const UObject* WorldContextObject)
{
	const UUserSettingsManager* userSettingsManager = GetUserSettingsManager(WorldContextObject);
	if (!userSettingsManager) return FControlsUserSettings();
	return userSettingsManager->GetControlsSettings();
}

FAccessibilityUserSettings UUserSettingsManager::GetUserAccessibilitySettings(const UObject* WorldContextObject)
{
	const UUserSettingsManager* userSettingsManager = GetUserSettingsManager(WorldContextObject);
	if (!userSettingsManager) return FAccessibilityUserSettings();
	return userSettingsManager->GetAccessibilitySettings();
}

void UUserSettingsManager::BindOnUserSettingsLoaded(const FBindOnUserSettingsLoaded& BindOnUserSettingsLoaded,
	const UObject* WorldContextObject)
{
	GetUserSettingsManager(WorldContextObject)->OnUserSettingsLoaded.Add(BindOnUserSettingsLoaded);
}

FVector2D UUserSettingsManager::ApplySensitivity(const FVector2D& Input, const UObject* WorldContextObject)
{
	const FControlsUserSettings controlsSettings = GetUserControlsSettings(WorldContextObject);
	return FVector2D(Input.X * controlsSettings.SensitivityX, Input.Y * controlsSettings.SensitivityY);
}

EHoldToggleMethod UUserSettingsManager::GetHoldToggleMethod(const UInputAction* InputAction,
	const UObject* WorldContextObject)
{
	const UUserSettingsManager* settingsManager = GetUserSettingsManager(WorldContextObject);
	const ULocalPlayer* LocalPlayer = settingsManager->GetWorld()->GetFirstPlayerController()->GetLocalPlayer();
	const UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	const TArray<FEnhancedActionKeyMapping> matchingMappings = InputSystem->GetAllPlayerMappableActionKeyMappings().FilterByPredicate([InputAction](const FEnhancedActionKeyMapping& mapping) {
		return mapping.Action.GetFName().ToString().Equals(InputAction->GetFName().ToString());
	});
	checkf(matchingMappings.Num() == 1, TEXT("Could not find mapping for: %s"), *InputAction->GetFName().ToString());

	checkf(settingsManager->GetControlsSettings().ToggleableInputs.Contains(matchingMappings[0].GetMappingName()),
		TEXT("Attempted to apply hold/toggle settings on an input with no toggle support: %s"), *matchingMappings[0].GetMappingName().ToString());
	return settingsManager->GetControlsSettings().ToggleableInputs[matchingMappings[0].GetMappingName()].HoldOrToggle;
}

UGameUserSettings* UUserSettingsManager::GetGameUserSettings()
{
	if (GEngine != nullptr)
	{
		return GEngine->GameUserSettings;
	}
	return nullptr;
}

