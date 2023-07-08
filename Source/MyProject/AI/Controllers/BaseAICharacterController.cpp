// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAICharacterController.h"

#include "AI/Characters/AICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/AIComponents/AIPatrollingComponent.h"
#include "Perception/AISense_Sight.h"
#include "GameCodeTypes.h"

void ABaseAICharacterController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if(IsValid(InPawn))
	{
		checkf(InPawn->IsA<AAICharacter>(), TEXT("ABaseAICharacterController::SetPawn ABaseAICharacterController can possess only AICharacter"));
		CachedAICharacter = StaticCast<AAICharacter*>(InPawn);
		RunBehaviorTree(CachedAICharacter->GetBehaviorTree());
		SetupPatrolling();
	}
	else
	{
		CachedAICharacter = nullptr;
	}
}

void ABaseAICharacterController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	Super::ActorsPerceptionUpdated(UpdatedActors);

	if(!CachedAICharacter.IsValid())
	{
		return;
	}
	TryMoveToNextTarget();
	
}

void ABaseAICharacterController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if(!Result.IsSuccess())
	{
		return;
	}
	TryMoveToNextTarget();
}

void ABaseAICharacterController::SetupPatrolling()
{
	UAIPatrollingComponent* PatrollingComponent = CachedAICharacter->GetPatrollingComponent();
	if(PatrollingComponent->CanPatrol())
	{
		const FVector ClosestWayPoint = PatrollingComponent->SelectClosestWayPoint();
		if(IsValid(Blackboard))
		{
			Blackboard->SetValueAsVector(BB_NextLocation, ClosestWayPoint);
			Blackboard->SetValueAsObject(BB_CurrentTarget, nullptr);
		}
		bIsPatrolling = true;
	}
}

void ABaseAICharacterController::TryMoveToNextTarget()
{
	UAIPatrollingComponent* PatrollingComponent = CachedAICharacter->GetPatrollingComponent();
	AActor* ClosestActor = GetClosestSensedActor(UAISense_Sight::StaticClass());
	if(IsValid(ClosestActor))
	{
		if(IsValid(Blackboard))
		{
			Blackboard->SetValueAsObject(BB_CurrentTarget, ClosestActor);
			SetFocus(ClosestActor, EAIFocusPriority::Gameplay);
		}
		bIsPatrolling = false;
	}
	else if (PatrollingComponent->CanPatrol())
	{
		const FVector WayPoint = bIsPatrolling ? PatrollingComponent->SelectNextWayPoint() : PatrollingComponent->SelectClosestWayPoint();
		if(IsValid(Blackboard))
		{
			ClearFocus(EAIFocusPriority::Gameplay);
			Blackboard->SetValueAsVector(BB_NextLocation, WayPoint);
			Blackboard->SetValueAsObject(BB_CurrentTarget, nullptr);
		}
		bIsPatrolling = true;
	}
}

bool ABaseAICharacterController::IsTargetReached(FVector TargetLocation) const
{
	return (TargetLocation - CachedAICharacter->GetActorLocation()).SizeSquared() <= FMath::Square(TargetReachRadius);
}
