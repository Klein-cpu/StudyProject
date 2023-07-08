// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseAIController.h"
#include "AITurretController.generated.h"

class ATurret;
/**
 * 
 */
UCLASS()
class MYPROJECT_API AAITurretController : public ABaseAIController
{
	GENERATED_BODY()

public:
	virtual void SetPawn(APawn* InPawn) override;

	virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;

	UFUNCTION()
	void OnTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	
private:
	TWeakObjectPtr<ATurret> CachedTurret;

};
