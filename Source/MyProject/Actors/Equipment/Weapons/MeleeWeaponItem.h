// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCodeTypes.h"
#include "Actors/Equipment/EquipableItem.h"
#include "MeleeWeaponItem.generated.h"

class UMeleeHitRegistrator;
USTRUCT(BlueprintType)
struct FMeleeAttackDescription
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee attack")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee attack", meta = (ClampMin = 0.f, UIMin = 0.f))
	float DamageAmount = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee attack")
	UAnimMontage* AttackMontage;
	
};
/**
 * 
 */
UCLASS(Blueprintable)
class MYPROJECT_API AMeleeWeaponItem : public AEquipableItem
{
	GENERATED_BODY()

public:
	AMeleeWeaponItem();

	void StartAttack(EMeleeAttackTypes AttackType);

	void SetIsHitRegistrationEnabled(bool bIsRegistrationEnabled_In);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee attack")
	TMap<EMeleeAttackTypes, FMeleeAttackDescription> Attacks;

private:
	UFUNCTION()
	void ProcessHit(const FHitResult& HitResult, const FVector& HitDirection);

	TArray<UMeleeHitRegistrator*> HitRegistrators;

	UPROPERTY()
	TSet<AActor*> HitActors;
	
	FMeleeAttackDescription* CurrentAttack;
	
	void OnAttackTimerElapsed();
	FTimerHandle AttackTimer;
	
};
