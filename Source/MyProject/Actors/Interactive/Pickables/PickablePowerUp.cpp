// Fill out your copyright notice in the Description page of Project Settings.


#include "PickablePowerUp.h"

#include "GameCodeTypes.h"
#include "Inventory/Items/InventoryItem.h"
#include "Characters/BaseCharacter.h"
#include "Utils/BaseDataTableUtils.h"

APickablePowerUp::APickablePowerUp()
{
	PowerUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PowerUpMesh"));
	SetRootComponent(PowerUpMesh);
}

void APickablePowerUp::Interact(ABaseCharacter* Character)
{
	FItemTableRow* ItemData = BaseDataTableUtils::FindInventoryItemData(GetDataTableID());

	if(ItemData == nullptr)
	{
		return;
	}

	TWeakObjectPtr<UInventoryItem> Item = TWeakObjectPtr<UInventoryItem>(NewObject<UInventoryItem>(Character, ItemData->InventoryItemClass)).Get();
	Item->Initialize(DataTableID, ItemData->InventoryItemDescription);

	const bool bPickedUp = Character->PickUpItem(Item);
	if(bPickedUp)
	{
		Destroy();
	}
}

FName APickablePowerUp::GetActionEventName() const
{
	return ActionInteract;
}
