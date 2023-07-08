// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipableItem.h"
#include "Characters/BaseCharacter.h"

AEquipableItem::AEquipableItem()
{
	SetReplicates(true);
}

void AEquipableItem::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	if(IsValid(NewOwner))
	{
		checkf(GetOwner()->IsA<ABaseCharacter>(),
		   TEXT("AEquipableItem::SetOwner only character can be an owner of an equipable item"));
		CachedCharacterOwner = StaticCast<ABaseCharacter*>(NewOwner);
		if(GetLocalRole() == ROLE_Authority)
		{
			SetAutonomousProxy(true);
		}
	}
	else
	{
		CachedCharacterOwner = nullptr;
	}
}

EEquipableItemType AEquipableItem::GetItemType()
{
	return ItemType;
}

UAnimMontage* AEquipableItem::GetCharacterEquipAnimMontage() const
{
	return CharacterEquipAnimMontage;
}

FName AEquipableItem::GetUnEquippedSocketName() const
{
	return UnEquippedSocketName;
}

FName AEquipableItem::GetEquippedSocketName() const
{
	return EquippedSocketName;
}

void AEquipableItem::Equip()
{
	if(OnEquipmentStateChanged.IsBound())
	{
		OnEquipmentStateChanged.Broadcast(true);
	}
}

void AEquipableItem::UnEquip()
{
	if(OnEquipmentStateChanged.IsBound())
	{
		OnEquipmentStateChanged.Broadcast(false);
	}
}

EReticleType AEquipableItem::GetReticleType() const
{
	return ReticleType;
}

FName AEquipableItem::GetDataTableID() const
{
	return DataTableID;
}

bool AEquipableItem::IsSlotCompatible(EEquipmentSlots Slot)
{
	return CompatibleEquipmentSlots.Contains(Slot);
}

ABaseCharacter* AEquipableItem::GetCharacterOwner() const
{
	return CachedCharacterOwner.IsValid() ? CachedCharacterOwner.Get() : nullptr;
}
