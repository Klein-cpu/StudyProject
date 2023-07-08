// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"

#include "Components/AIComponents/AIPatrollingComponent.h"

AAICharacter::AAICharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PatrollingComponent = CreateDefaultSubobject<UAIPatrollingComponent>(TEXT("AIPatrolling"));
}

UAIPatrollingComponent* AAICharacter::GetPatrollingComponent() const
{
	return PatrollingComponent;
}

UBehaviorTree* AAICharacter::GetBehaviorTree() const
{
	return BehaviorTree;
}
