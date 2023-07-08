// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCodeTypes.h"
#include "GameFramework/Actor.h"
#include "EquipableItem.generated.h"

class ABaseCharacter;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentStateChanged, bool, bIsEquipped);

class UAnimMontage;
UCLASS(Abstract, NotBlueprintable)
class MYPROJECT_API AEquipableItem : public AActor
{
	GENERATED_BODY()

public:
	AEquipableItem();
	
	virtual void SetOwner(AActor* NewOwner) override;
	
	EEquipableItemType GetItemType();

	UAnimMontage* GetCharacterEquipAnimMontage() const;
	
	FName GetUnEquippedSocketName() const;
	FName GetEquippedSocketName() const;

	virtual void Equip();
	virtual void UnEquip();

	virtual EReticleType GetReticleType() const;

	FName GetDataTableID() const;

	bool IsSlotCompatible(EEquipmentSlots Slot);
	
protected:
	UPROPERTY(BlueprintAssignable)
	FOnEquipmentStateChanged OnEquipmentStateChanged;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipable item")
	EEquipableItemType ItemType = EEquipableItemType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipable item")
	UAnimMontage* CharacterEquipAnimMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipable item")
	FName UnEquippedSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipable item")
	FName EquippedSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipable item")
	TArray<EEquipmentSlots> CompatibleEquipmentSlots;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reticle")
	EReticleType ReticleType = EReticleType::None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName DataTableID = NAME_None;
	
	ABaseCharacter* GetCharacterOwner() const;
	
private:
		TWeakObjectPtr<ABaseCharacter> CachedCharacterOwner;
};
