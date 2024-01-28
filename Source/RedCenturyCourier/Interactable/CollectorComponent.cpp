// Fill out your copyright notice in the Description page of Project Settings.


#include "CollectorComponent.h"

#include "CollectableItem.h"
#include "Camera/CameraComponent.h"


UCollectorComponent::UCollectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	CollectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphere"));
	CollectionSphere->InitSphereRadius(150.f);
	static FName CollisionProfileName(TEXT("OverlapAllDynamic"));
	CollectionSphere->SetCollisionProfileName(CollisionProfileName);
	// CollectionSphere->CanCharacterStepUpOn = ECB_No;
	CollectionSphere->SetGenerateOverlapEvents(true);
	CollectionSphere->SetupAttachment(this);

	PrimeCollectable = nullptr;
	
	bRankByCamLook = false;
	MaxLookAngleForRanking = 3.f;
	bRankByProximity = true;
}


void UCollectorComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CollectionSphere->UpdateOverlaps();
}


void UCollectorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UCameraComponent* Cam = nullptr;
	if (bRankByCamLook)
	{
		Cam = GetOwner()->FindComponentByClass<UCameraComponent>();
	}

	const auto RateMyCollectable = [this, Cam](const AActor* CollectableItem)
	{
		if (bRankByCamLook)
		{
			// TODO this requires complex screen/world space projection since I want this to detect objects in a radius around the 'crosshair'
			UE_LOG(LogTemp, Warning, TEXT("bRankByCamLook is not yet implemented and does nothing!"));
			// UGameplayStatics::ProjectWorldToScreen()
			// Cam->
		}
		if (bRankByProximity)
		{
			return static_cast<uint8>(FMath::Floor(FVector::Dist(CollectableItem->GetActorLocation(), GetComponentLocation())));
		}
		return UINT8_MAX;
	};
	
	TArray<AActor*> OverlapItems;
	CollectionSphere->GetOverlappingActors(OverlapItems, ACollectableItem::StaticClass());
	PrimeCollectable = nullptr;
	uint8 PrimeRank = UINT8_MAX;
	for (AActor* OverlapItem : OverlapItems)
	{
		ACollectableItem* CollectableItem = Cast<ACollectableItem>(OverlapItem);
		// In theory, this should be impossible since GetOverlappingActors is filtering by class, but it doesn't cast
		// implicitly, so better safe than sorry I guess.
		if (CollectableItem == nullptr)
			continue;

		// If owner or item has another overlap primitive which is bigger than either of the collection spheres, it
		// could erroneously filter in. Need to check the distance is within a sane range.
		if (FVector::Dist(CollectableItem->GetActorLocation(), GetComponentLocation())
			> CollectionSphere->GetScaledSphereRadius() + CollectableItem->CollectableSphere->GetScaledSphereRadius())
			continue;

		if (CollectableItem->GetAttachParentActor() != nullptr)
			continue;
		
		const uint8 ItemRank = RateMyCollectable(CollectableItem);
		if (PrimeCollectable == nullptr || ItemRank < PrimeRank)
		{
			PrimeRank = ItemRank;
			PrimeCollectable = CollectableItem;
		}
	}
}

