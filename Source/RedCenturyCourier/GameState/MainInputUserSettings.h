// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "PlayerAxisSettings.h"
#include "MainInputUserSettings.generated.h"



/**
 * TODO emlings doc this
 */
UCLASS()
class REDCENTURYCOURIER_API UMainInputUserSettings : public UEnhancedInputUserSettings
{
	GENERATED_BODY()

public:

	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End UObject Interface
	
	/**
	 * Sets the axis properties for the given axis input (no profile association).
	*/
	UFUNCTION(BlueprintCallable, Category="Enhanced Input|User Settings", meta = (AutoCreateRefTerm = "FailureReason"))
	virtual void SetAxisProperties(const FName MappingName, const FAxisProperties& InProperties, FGameplayTagContainer& FailureReason);
	
	UFUNCTION(BlueprintPure, Category="Enhanced Input|User Settings")
	virtual bool HasAxisProperties(const FName MappingName) const;
	
	UFUNCTION(BlueprintPure, Category="Enhanced Input|User Settings")
	virtual FAxisProperties GetAxisProperties(const FName MappingName) const;

protected:
	/**
	 * A map of "Axis Properties", containing custom properties relating to an axis (such as sensitivity)
	 * Note: Dirty axis properties will be serialized from UMainInputUserSettings::Serialize
	 */
	UPROPERTY(BlueprintReadOnly, Transient, EditAnywhere, Category="Enhanced Input|User Settings")
	TMap<FName, FAxisProperties> SavedAxisProperties;
};
