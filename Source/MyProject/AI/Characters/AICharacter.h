// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "AICharacter.generated.h"


class UBehaviorTree;
class UAIPatrollingComponent;
UCLASS(Blueprintable)
class MYPROJECT_API AAICharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AAICharacter(const FObjectInitializer& ObjectInitializer);

	UAIPatrollingComponent* GetPatrollingComponent() const;

	UBehaviorTree* GetBehaviorTree() const;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAIPatrollingComponent* PatrollingComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	UBehaviorTree* BehaviorTree;
	
};
