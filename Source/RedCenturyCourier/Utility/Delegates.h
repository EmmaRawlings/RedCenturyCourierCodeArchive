// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Delegates.generated.h"

// DOCSONLY: namespace Utility {
/*
 * Home of ALL Delegate definitions. Prevents delegate duplication and ensures proper utilisation of interfaces
 * (instead of passing around whole classes on delegate trigger).
 */

// /*
//  * SIMPLE
//  */
// DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAction);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionWithCost, int32, Cost);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFire, UAnimMontage*, FireAnimation);
// DECLARE_DYNAMIC_DELEGATE_RetVal(FVector, FFireOriginObserver);
// DECLARE_DYNAMIC_DELEGATE_RetVal(FRotator, FFireDirectionObserver);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOut, UObject*, Object);

// /*
//  * ON COLLECT
//  */
// UINTERFACE(MinimalAPI, Blueprintable)
// class UCollectableInterface : public UInterface {
// 	GENERATED_BODY()
// };
// class REDCENTURYCOURIER_API ICollectableInterface
// {
// 	GENERATED_BODY()
// };
//
// UINTERFACE(MinimalAPI, Blueprintable)
// class UCollectorInterface : public UInterface {
// 	GENERATED_BODY()
// };
// class REDCENTURYCOURIER_API ICollectorInterface
// {
// 	GENERATED_BODY()
// public:
// 	/** Inform the collector that it has crossed a collectable object. It may choose to reject the collectable by
// 		returning false, or inform the caller that it has been collected by returning true. */
// 	virtual bool Collect(TScriptInterface<ICollectableInterface> Collectable) = 0;
// };
//
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollected, TScriptInterface<ICollectorInterface>, Collector);
// DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOnCollection, TScriptInterface<ICollectableInterface>, Collectable);
//
// /*
//  * TRIGGER ZONE
//  */
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorTriggeredZone, AActor*, OtherActor);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorWithinZone, AActor*, OtherActor, float, DeltaTime);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComponentTriggeredZone, UActorComponent*, OtherComp);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComponentWithinZone, UActorComponent*, OtherComp, float, DeltaTime);
// DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(UActorComponent*, FFilterComponent, UPrimitiveComponent*, OtherComp);

UCLASS()
class REDCENTURYCOURIER_API UDelegates final : public UObject
{
	GENERATED_BODY()
private:
	UDelegates();
};

// /*
//  * INVENTORY
//  */
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemAdded, AActor*, ItemActor, int32, Index, bool, bIsNewEntry);

/*
 * INPUT
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnToggleableInputAction, ETriggerEvent, TriggerEvent, bool, ActionValue, float, ElapsedSeconds);

// DOCSONLY: }
