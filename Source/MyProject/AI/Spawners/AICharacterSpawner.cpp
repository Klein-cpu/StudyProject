// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacterSpawner.h"

#include "Actors/Interactive/Interface/IInteractable.h"
#include "AI/Characters/AICharacter.h"

AAICharacterSpawner::AAICharacterSpawner()
{
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnRoot"));
	SetRootComponent(SceneRoot);
}

void AAICharacterSpawner::SpawnAI()
{
	if(!bCanSpawn || !IsValid(CharacterClass))
	{
		return;	
	}

	AAICharacter* AICharacter = GetWorld()->SpawnActor<AAICharacter>(CharacterClass, GetTransform());
	if(!IsValid(AICharacter->Controller))
	{
		AICharacter->SpawnDefaultController();
	}

	if(bDoOnce)
	{
		UnSubscribeFromTrigger();
		bCanSpawn = false;
	}
}

void AAICharacterSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if(PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_STRING_CHECKED(AAICharacterSpawner, SpawnTriggerActor))
	{
		SpawnTrigger = SpawnTriggerActor;
		if(SpawnTrigger.GetInterface())
		{
			if(!SpawnTrigger->HasOnInteractionCallback())
			{
				SpawnTriggerActor = nullptr;
				SpawnTrigger = nullptr;
			}
		}
		else
		{
			SpawnTriggerActor = nullptr;
			SpawnTrigger = nullptr;
		}
	}
}

void AAICharacterSpawner::BeginPlay()
{
	Super::BeginPlay();

	if(SpawnTrigger.GetInterface())
	{
		TriggerHandle = SpawnTrigger->AddOnInteractionUFunction(this, FName("SpawnAI"));
	}
	
	if(bIsSpawnedOnStart)
	{
		SpawnAI();
	}
}

void AAICharacterSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnSubscribeFromTrigger();
	
	Super::EndPlay(EndPlayReason);
}

void AAICharacterSpawner::UnSubscribeFromTrigger()
{
	if(TriggerHandle.IsValid() && SpawnTrigger.GetInterface())
	{
		SpawnTrigger->RemoveOnInteractionDelegate(TriggerHandle);
	}
}
