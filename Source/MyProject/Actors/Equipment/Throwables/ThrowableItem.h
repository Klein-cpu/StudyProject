// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Equipment/EquipableItem.h"
#include "ThrowableItem.generated.h"

/**
 * 
 */
class AProjectile;
UCLASS(Blueprintable)
class MYPROJECT_API AThrowableItem : public AEquipableItem
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void Throw(AProjectile* Projectile);

	int32 GetAmountOfItems();
	void AddItems(int32 Amount);

	EAmunitionType GetThrowableItemType();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Throwables", meta = (UIMin = -90.f, UIMax = 90.f, ClampMin = -90.f, ClampMax = 90.f))
	float ThrowAngle = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwables | Parameters | Item")
	EAmunitionType ThrowableItemType ;

private:
	UPROPERTY(Replicated)
	int32 AvailableItems = 0;
};
