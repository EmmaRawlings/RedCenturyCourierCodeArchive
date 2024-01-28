// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CollectableItem.h"
#include "ToolItem.generated.h"

UCLASS()
class REDCENTURYCOURIER_API AToolItem : public ACollectableItem
{
	GENERATED_BODY()

public:
	AToolItem();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tool Item")
	bool bActionActive;
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Tool Item")
	// ReSharper disable once CppUEBlueprintCallableFunctionUnused
	// ReSharper disable once CppUEBlueprintImplementableEventNotImplemented
	void ActionOnce();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Tool Item")
	// ReSharper disable once CppUEBlueprintCallableFunctionUnused
	// ReSharper disable once CppUEBlueprintImplementableEventNotImplemented
	void StartAction();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Tool Item")
	// ReSharper disable once CppUEBlueprintCallableFunctionUnused
	// ReSharper disable once CppUEBlueprintImplementableEventNotImplemented
	void StopAction();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
