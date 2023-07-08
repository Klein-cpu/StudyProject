// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controllers/BaseAIController.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

ABaseAIController::ABaseAIController()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
}

AActor* ABaseAIController::GetClosestSensedActor(TSubclassOf<UAISense> SenseClass)
{
	if(!IsValid(GetPawn()))
	{
		return nullptr;
	}
	
	TArray<AActor*> DetectedActors;
	PerceptionComponent->GetCurrentlyPerceivedActors(SenseClass, DetectedActors);
	
	AActor* ClosestActor = nullptr;
	float MinSquaredDistance = FLT_MAX;
	const FVector TurretLocation = GetPawn()->GetActorLocation();
	
	for(auto DetectedActor : DetectedActors)
	{
		const float CurrentSquaredDistance = (TurretLocation-DetectedActor->GetActorLocation()).SizeSquared();
		if(CurrentSquaredDistance < MinSquaredDistance)
		{
			MinSquaredDistance = CurrentSquaredDistance;
			ClosestActor = DetectedActor;
		}
	}
	return ClosestActor;
}
