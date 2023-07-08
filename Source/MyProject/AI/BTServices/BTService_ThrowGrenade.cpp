// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTServices/BTService_ThrowGrenade.h"

#include "AIController.h"
#include "Actors/Equipment/Throwables/ThrowableItem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/BaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

UBTService_ThrowGrenade::UBTService_ThrowGrenade()
{
	NodeName = "ThrowGrenade";
}

void UBTService_ThrowGrenade::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if(!IsValid(AIController) || !IsValid(Blackboard))
	{
		return;
	}

	ABaseCharacter* Character = Cast<ABaseCharacter>(AIController->GetPawn());
	if(!IsValid(Character))
	{
		return;
	}
	
	AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName));
	if(!IsValid(CurrentTarget))
	{
		return;
	}
	
	float DistSq = FVector::DistSquared(CurrentTarget->GetActorLocation(), Character->GetActorLocation());
	if(DistSq > FMath::Square(MaxThrowDistance) && DistSq < FMath::Square(MinThrowDistance))
	{
		return;
	}

	Character->EquipPrimaryItem();
	
}
