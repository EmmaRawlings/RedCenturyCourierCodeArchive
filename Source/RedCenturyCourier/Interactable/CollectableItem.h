// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "CollectableItem.generated.h"

UCLASS()
class REDCENTURYCOURIER_API ACollectableItem : public AActor
{
	GENERATED_BODY()

public:
	ACollectableItem();
	UPROPERTY(Category="Collectable Item", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY(Category="Collectable Item", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> CollectableSphere;
	UPROPERTY(Category="Collectable Item", EditAnywhere, BlueprintReadWrite)
	FString ItemType;
	UPROPERTY(Category="Collectable Item", EditAnywhere, BlueprintReadWrite)
	FString ItemName;
	// UPROPERTY(Category="Collectable Item", EditAnywhere, BlueprintReadWrite)
	// bool bItemAvailable;
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Collectable Item")
	// ReSharper disable once CppUEBlueprintCallableFunctionUnused
	// ReSharper disable once CppUEBlueprintImplementableEventNotImplemented
	void Collect(AActor* CollectingActor);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Collectable Item")
	// ReSharper disable once CppUEBlueprintCallableFunctionUnused
	// ReSharper disable once CppUEBlueprintImplementableEventNotImplemented
	void Discard(AActor* DiscardingActor);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Collectable Item")
	// ReSharper disable once CppUEBlueprintCallableFunctionUnused
	// ReSharper disable once CppUEBlueprintImplementableEventNotImplemented
	bool IsCollectable(AActor* CollectingActor);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
