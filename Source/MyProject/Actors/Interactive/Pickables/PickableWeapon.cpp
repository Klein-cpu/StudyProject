// Fill out your copyright notice in the Description page of Project Settings.


#include "PickableWeapon.h"

#include "GameCodeTypes.h"
#include "Characters/BaseCharacter.h"
#include "Engine/DataTable.h"
#include "Inventory/Items/InventoryItem.h"
#include "Inventory/Items/Equipables/WeaponInventoryItem.h"
#include "Utils/BaseDataTableUtils.h"

APickableWeapon::APickableWeapon()
{
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
}

void APickableWeapon::Interact(ABaseCharacter* Character)
{
	FWeaponTableRow* WeaponRow = BaseDataTableUtils::FindWeaponData(DataTableID);
	if(WeaponRow)
	{
		TWeakObjectPtr<UWeaponInventoryItem> Weapon = NewObject<UWeaponInventoryItem>(Character);
		Weapon->Initialize(DataTableID, WeaponRow->WeaponItemDescription);
		Weapon->SetEquipWeaponClass(WeaponRow->EquipableActor);
		Character->PickUpItem(Weapon);
		Destroy();
	}
}

FName APickableWeapon::GetActionEventName() const
{
	return ActionInteract;
}
