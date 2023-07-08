// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_ThrowGrenade.generated.h"

class AThrowableItem;
/**
 * 
 */
UCLASS()
class MYPROJECT_API UBTService_ThrowGrenade : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_ThrowGrenade();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	FBlackboardKeySelector TargetKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float MinThrowDistance = 800.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float MaxThrowDistance = 1000.f;
};
