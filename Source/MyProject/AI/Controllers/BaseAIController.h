// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseAIController.generated.h"

class UAISense;
/**
 * 
 */
UCLASS()
class MYPROJECT_API ABaseAIController : public AAIController
{
	GENERATED_BODY()

public:
	ABaseAIController();

protected:
	AActor* GetClosestSensedActor(TSubclassOf<UAISense> SenseClass);
	
};
