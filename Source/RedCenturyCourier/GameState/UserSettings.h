// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "InputTriggers.h"

#include "UserSettings.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUserSettings, Log, All);

// DOCSONLY: namespace GameState {
/** Video Settings */
USTRUCT(BlueprintType)
struct FVideoUserSettings
{
	GENERATED_BODY()

	/** Field of View */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings",
		meta = (UIMin = 60.f, UIMax = 130.f, ClampMin = 60.f, ClampMax = 130.f))
	float FieldOfView;
	// TODO alpha/brightness
	// TODO viewmodel fov + other viewmodel settings
	// TODO various quality settings
};

/** Audio Channel */
USTRUCT(BlueprintType)
struct FAudioChannel
{
	GENERATED_BODY()
	
	/** Name */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	FString Name;
	/** Level */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings",
		meta = (UIMin = 0.f, UIMax = 1.f, ClampMin = 0.f, ClampMax = 1.f))
	float Level;

	static FAudioChannel BuildNew(const FString& Name, const float& Level) {
		FAudioChannel Result;
		Result.Name = Name;
		Result.Level = Level;
		return Result;
	}
};

/** Audio Settings */
USTRUCT(BlueprintType)
struct FAudioUserSettings
{
	GENERATED_BODY()

	/** Audio Channels */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	TArray<FAudioChannel> AudioChannels;
	/** Mute in Background */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	bool bMuteInBackground;
	// TODO dynamic range
};

/** Input Binding Keys */
USTRUCT(BlueprintType)
struct FInputBindingKeys
{
	GENERATED_BODY()
	/** Keys */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	TArray<FKey> Keys;

	static FInputBindingKeys BuildNew(const TArray<FKey>& Keys) {
		FInputBindingKeys Result;
		Result.Keys = Keys;
		return Result;
	}
};

UENUM(BlueprintType)
enum EHoldToggleMethod
{
	Hold, Toggle, Contextual
};

/** Toggleable Input */
USTRUCT(BlueprintType)
struct FToggleableInput
{
	GENERATED_BODY()
	/** Hold Or Toggle */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	TEnumAsByte<EHoldToggleMethod> HoldOrToggle;

	static FToggleableInput BuildNew(TEnumAsByte<EHoldToggleMethod> HoldOrToggle) {
		FToggleableInput Result;
		Result.HoldOrToggle = HoldOrToggle;
		return Result;
	}
};

/** Controls Settings */
USTRUCT(BlueprintType)
struct FControlsUserSettings
{
	GENERATED_BODY()
	
	/** Sensitivity X */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings",
		meta = (UIMin = 0.f, UIMax = 100.f, ClampMin = 0.f, ClampMax = 100.f))
	float SensitivityX;
	/** Sensitivity Y */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings",
		meta = (UIMin = 0.f, UIMax = 100.f, ClampMin = 0.f, ClampMax = 100.f))
	float SensitivityY;
	
	/** Hold Jump To Wall Run/Climb */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	bool bHoldJumpToWallRunClimb;
	
	/** Hold Jump To Ledge Vault */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	bool bHoldJumpToLedgeVault;

	// TODO aim assist
	// TODO controller
	
	UPROPERTY()
	FString _BindingsHelp = TEXT("Valid keys: https://docs.unrealengine.com/API/Runtime/InputCore/EKeys#constants");
	/** Bindings */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	TMap<FName, FInputBindingKeys> Bindings;
	UPROPERTY()
	FString _HoldToggleHelp = TEXT("Valid Hold/Toggle methods: Hold, Toggle, Contextual");
	/** Toggleable Inputs */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	TMap<FName, FToggleableInput> ToggleableInputs;
	// TODO bind to button combos?
};

/** Accessibility Settings */
USTRUCT(BlueprintType)
struct FAccessibilityUserSettings
{
	GENERATED_BODY()
	
	// TODO closed captions, full subtitles, etc
	// TODO notify sounds & direction
	/** Screen Shake Intensity */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings",
		meta = (UIMin = 0.f, UIMax = 1.f, ClampMin = 0.f, ClampMax = 1.f))
	float ScreenShakeIntensity;
	/** Screen Tilt Strength */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings",
		meta = (UIMin = 0.f, UIMax = 1.f, ClampMin = 0.f, ClampMax = 1.f))
	float ScreenTiltStrength;
	/** Camera Motion Strength */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings",
		meta = (UIMin = 0.f, UIMax = 1.f, ClampMin = 0.f, ClampMax = 1.f))
	float CameraMotionStrength;
	/** Iteractable Highlight */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	FColor IteractableHighlight;
	/** Enemy Highlight */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	FColor EnemyHighlight;
	/** Ally Highlight */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	FColor AllyHighlight;
	// TODO projectile/attack highlight/notifications
	// TODO colour blind settings
	// TODO language/localisation
	// TODO HUD settings (centralise, opacity, size, etc)
};

/** Settings */
USTRUCT(BlueprintType)
struct FUserSettings
{
	GENERATED_BODY()

	/** Video Settings */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	FVideoUserSettings VideoSettings;
	/** Audio Settings */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	FAudioUserSettings AudioSettings;
	/** Controls Settings */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	FControlsUserSettings ControlsSettings;
	/** Accessibility Settings */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	FAccessibilityUserSettings AccessibilitySettings;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUserSettingsLoaded, FUserSettings, UserSettings);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBindOnUserSettingsLoaded, FUserSettings, UserSettings);
class UBoundToggleableInputAction;

/** Manages the user settings for the game, can save/load settings, and provide access to the loaded settings struct.
 * Ideally, there should only be one SettingsManagerComponent at a time (this won't be validated). */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REDCENTURYCOURIER_API UUserSettingsManager : public UObject
{
	GENERATED_BODY()
public:	
	UUserSettingsManager();
	virtual void Init();
	/** Settings structure for the game.
	 * Initially, this will be empty, call Load() to populate from file, or add the settings you need. */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	FUserSettings Settings;
	/** Relative file path for the settings file. The base directory is the config directory for the project. */
	UPROPERTY(EditAnyWhere, BluePrintReadWrite, Category = "User Settings")
	FString FilePathRel;
	/** Load the settings file into the Settings property (use FilePathRel property to configure file path).
	 * @returns whether the settings file was successfully loaded */
	UFUNCTION(BlueprintCallable, Category = "User Settings")
	bool Load();
	/** Save the settings file, based on the Settings property (use FilePathRel property to configure file path).
	 * @returns whether the settings file was successfully saved */
	UFUNCTION(BlueprintCallable, Category = "User Settings")
	bool Save();
	void Apply();
	UPROPERTY(EditAnyWhere, BlueprintAssignable, Category = "User Settings")
	FOnUserSettingsLoaded OnUserSettingsLoaded;

	// Blueprint helpers
	/** Video settings for the game.
	 * Initially, this will be empty, call Load() to populate from file, or add the settings you need. */
	UFUNCTION(BlueprintPure, Category = "User Settings")
	FVideoUserSettings GetVideoSettings() const;
	/** Audio settings for the game.
	 * Initially, this will be empty, call Load() to populate from file, or add the settings you need. */
	UFUNCTION(BlueprintPure, Category = "User Settings")
	FAudioUserSettings GetAudioSettings() const;
	/** Controls settings for the game.
	 * Initially, this will be empty, call Load() to populate from file, or add the settings you need. */
	UFUNCTION(BlueprintPure, Category = "User Settings")
	FControlsUserSettings GetControlsSettings() const;
	/** Accessibility settings for the game.
	 * Initially, this will be empty, call Load() to populate from file, or add the settings you need. */
	UFUNCTION(BlueprintPure, Category = "User Settings")
	FAccessibilityUserSettings GetAccessibilitySettings() const;
	/** Set new video settings for the game. */
	UFUNCTION(BlueprintCallable, Category = "User Settings")
	FVideoUserSettings SetVideoSettings(const FVideoUserSettings& NewSettings);
	/** Set new audio settings for the game. */
	UFUNCTION(BlueprintCallable, Category = "User Settings")
	FAudioUserSettings SetAudioSettings(const FAudioUserSettings& NewSettings);
	/** Set new controls settings for the game. */
	UFUNCTION(BlueprintCallable, Category = "User Settings")
	FControlsUserSettings SetControlsSettings(const FControlsUserSettings& NewSettings);
	/** Set new accessibility settings for the game. */
	UFUNCTION(BlueprintCallable, Category = "User Settings")
	FAccessibilityUserSettings SetAccessibilitySettings(const FAccessibilityUserSettings& NewSettings);
	/** Retrieve the User Settings Manager. */
	UFUNCTION(BlueprintPure, Category="User Settings", meta=(WorldContext="WorldContextObject"))
	static class UUserSettingsManager* GetUserSettingsManager(const UObject* WorldContextObject);
	/** Video settings for the game.
	 * Initially, this will be empty, call Load() to populate from file, or add the settings you need. */
	UFUNCTION(BlueprintPure, Category = "User Settings", meta=(WorldContext="WorldContextObject"))
	static FVideoUserSettings GetUserVideoSettings(const UObject* WorldContextObject);
	/** Audio settings for the game.
	 * Initially, this will be empty, call Load() to populate from file, or add the settings you need. */
	UFUNCTION(BlueprintPure, Category = "User Settings", meta=(WorldContext="WorldContextObject"))
	static FAudioUserSettings GetUserAudioSettings(const UObject* WorldContextObject);
	/** Controls settings for the game.
	 * Initially, this will be empty, call Load() to populate from file, or add the settings you need. */
	UFUNCTION(BlueprintPure, Category = "User Settings", meta=(WorldContext="WorldContextObject"))
	static FControlsUserSettings GetUserControlsSettings(const UObject* WorldContextObject);
	/** Accessibility settings for the game.
	 * Initially, this will be empty, call Load() to populate from file, or add the settings you need. */
	UFUNCTION(BlueprintPure, Category = "User Settings", meta=(WorldContext="WorldContextObject"))
	static FAccessibilityUserSettings GetUserAccessibilitySettings(const UObject* WorldContextObject);
	
	/** Bind OnUserSettingsLoaded. */
	UFUNCTION(BlueprintPure, Category="User Settings", meta=(WorldContext="WorldContextObject"))
	static void BindOnUserSettingsLoaded(const FBindOnUserSettingsLoaded& BindOnUserSettingsLoaded,
		const UObject* WorldContextObject);
	
	/** Applies sensitivity as a scalar to the given input vector.
	 * TODO will also do other stuff like acceleration/aim assist in future */
	UFUNCTION(BlueprintPure, Category = "User Settings", meta=(WorldContext="WorldContextObject"))
	static FVector2D ApplySensitivity(const FVector2D& Input, const UObject* WorldContextObject);

	/** Retrieve the hold/toggle method for the input. */
	UFUNCTION(BlueprintPure, Category = "User Settings", meta=(WorldContext="WorldContextObject"))
	static EHoldToggleMethod GetHoldToggleMethod(const UInputAction* InputAction, const UObject* WorldContextObject);
private:
	static UGameUserSettings* GetGameUserSettings();
};
// DOCSONLY: }

// TODO ETriggerEvent: enum class ETriggerEvent : uint8
