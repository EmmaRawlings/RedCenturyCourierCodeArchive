// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PlayerAxisSettings.h"
#include "MainGameInstance.generated.h"

class UUserSettingsManager;
// DOCSONLY: namespace GameState {
/**
 * Maintains global state for the entire game, across every scene. User settings can be found here.
 */
UCLASS()
class REDCENTURYCOURIER_API UMainGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UMainGameInstance();
	/** User Settings manager for the game. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Main Game Instance")
	TObjectPtr<UUserSettingsManager> UserSettingsManager;
	virtual void Init() override;
	/** Retrieve the Game Instance. */
	UFUNCTION(BlueprintPure, Category="Main Game Instance", meta=(WorldContext="WorldContextObject"))
	static class UMainGameInstance* GetGameInstance(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintPure, Category="Main Game Instance")
	virtual bool HasDefaultAxisProperties(const FName MappingName) const;
	
	UFUNCTION(BlueprintPure, Category="Main Game Instance")
	virtual FAxisProperties GetDefaultAxisProperties(const FName MappingName) const;
protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Main Game Instance")
	TMap<FName, FAxisProperties> DefaultAxisProperties;
};
// DOCSONLY: }
