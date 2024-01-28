// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CollectableItem.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "CollectorComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class REDCENTURYCOURIER_API UCollectorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UCollectorComponent();
	
	UPROPERTY(Category="Collector Component", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> CollectionSphere;
	
	UPROPERTY(Category="Collector Component", EditAnywhere, BlueprintReadWrite)
	bool bRankByCamLook;

	UPROPERTY(Category="Collector Component", EditAnywhere, BlueprintReadWrite)
	float MaxLookAngleForRanking;
	
	UPROPERTY(Category="Collector Component", EditAnywhere, BlueprintReadWrite)
	bool bRankByProximity;
	
	UPROPERTY(Category="Collector Component", EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ACollectableItem> PrimeCollectable;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
