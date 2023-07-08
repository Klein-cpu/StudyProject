// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterEquipmentComponent.h"

#include "GameCodeTypes.h"
#include "Actors/Equipment/Throwables/ThrowableItem.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Actors/Projectiles/Projectile.h"
#include "Blueprint/UserWidget.h"
#include "Characters/BaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "UI/Widget/Equipment/EquipmentViewWidget.h"

UCharacterEquipmentComponent::UCharacterEquipmentComponent()
{
	SetIsReplicatedByDefault(true);
}

void UCharacterEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCharacterEquipmentComponent, CurrentEquippedSlot);
	DOREPLIFETIME(UCharacterEquipmentComponent, AmmunitionArray);
	DOREPLIFETIME(UCharacterEquipmentComponent, ItemsArray);
}

EEquipableItemType UCharacterEquipmentComponent::GetCurrentEquippedItemType() const
{
	EEquipableItemType Result = EEquipableItemType::None;
	if(IsValid(CurrentEquippedItem))
	{
		Result = CurrentEquippedItem->GetItemType();
	}
	return Result;
}

ARangeWeaponItem* UCharacterEquipmentComponent::GetCurrentRangeWeapon() const
{
	return CurrentRangeWeapon;
}

AMeleeWeaponItem* UCharacterEquipmentComponent::GetCurrentMeleeWeapon() const
{
	return CurrentMeleeWeapon;
}

AThrowableItem* UCharacterEquipmentComponent::GetCurrentThrowableItem() const
{
	return CurrentThrowableItem;
}

bool UCharacterEquipmentComponent::IsEquipping() const
{
	return bIsEquipping;
}

void UCharacterEquipmentComponent::EquipItemInSlot(EEquipmentSlots Slot)
{
	if(bIsEquipping)
	{
		return;
	}
	
	UnEquipCurrentItem();
	
	CurrentEquippedItem = ItemsArray[static_cast<uint32>(Slot)];
	CurrentRangeWeapon = Cast<ARangeWeaponItem>(CurrentEquippedItem);
	CurrentThrowableItem = Cast<AThrowableItem>(CurrentEquippedItem);
	CurrentMeleeWeapon = Cast<AMeleeWeaponItem>(CurrentEquippedItem);
	
	if(IsValid(CurrentThrowableItem) && CurrentThrowableItem->GetAmountOfItems() <= 0)
	{
		return;
	}
	
	if(IsValid(CurrentEquippedItem))
	{
		UAnimMontage* EquipMontage = CurrentEquippedItem->GetCharacterEquipAnimMontage();
		if(IsValid(EquipMontage))
		{
			bIsEquipping = true;
			float MontageDuration = CachedBaseCharacter->PlayAnimMontage(EquipMontage);
			GetWorld()->GetTimerManager().SetTimer(EquipTimer, this, &UCharacterEquipmentComponent::EquipAnimationFinished, MontageDuration, false);
		}
		else
		{
			AttachCurrentItemToEquippedSocket();
		}
		CurrentEquippedItem->Equip();
	}
	
	if(IsValid(CurrentRangeWeapon))
	{
		OnCurrentWeaponAmmoChangedHandle = CurrentRangeWeapon->OnAmmoChanged.AddUFunction(this, FName("OnCurrentWeaponAmmoChanged"));
		OnCurrentWeaponReloadedHandle = CurrentRangeWeapon->OnReloadComplete.AddUFunction(this, FName("OnWeaponReloadComplete"));
		OnCurrentWeaponAmmoChanged(CurrentRangeWeapon->GetAmmo());
	}

	if(OnEquippedItemChanged.IsBound())
	{
		OnEquippedItemChanged.Broadcast(CurrentEquippedItem);
	}
	
	CurrentEquippedSlot = Slot;
	if(GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_EquipItemInSlot(CurrentEquippedSlot);
	}
}

void UCharacterEquipmentComponent::EquipAnimationFinished()
{
	bIsEquipping = false;
}

void UCharacterEquipmentComponent::AttachCurrentItemToEquippedSocket()
{
	if(IsValid(CurrentEquippedItem))
	{
		CurrentEquippedItem->AttachToComponent(CachedBaseCharacter->GetMesh(),
											   FAttachmentTransformRules::KeepRelativeTransform,
											   CurrentEquippedItem->GetEquippedSocketName());
	}
}

void UCharacterEquipmentComponent::LaunchCurrentThrowableItem()
{
	if(GetOwnerRole() < ROLE_Authority)
	{
		return;
	}
	
	AProjectile* ThrowableItem = ThrowableItemPool[CurrentThrowableItemIndex];
	if(CurrentThrowableItem)
	{
		CurrentThrowableItem->Throw(ThrowableItem);
		ThrowableItem->SetProjectileActive(true);
		
		++CurrentThrowableItemIndex;
		if(CurrentThrowableItemIndex == ThrowableItemPool.Num())
		{
			CurrentThrowableItemIndex = 0;
		}
		
		OnCurrentThrowableItemAmountChanged(CurrentThrowableItem->GetAmountOfItems());
		bIsEquipping = false;
		EquipItemInSlot(PreviousEquippedSlot);
	}
}

void UCharacterEquipmentComponent::UnEquipCurrentItem()
{
	if(IsValid(CurrentEquippedItem))
	{
		CurrentEquippedItem->AttachToComponent(CachedBaseCharacter->GetMesh(),
											   FAttachmentTransformRules::KeepRelativeTransform,
											   CurrentEquippedItem->GetUnEquippedSocketName());
		CurrentEquippedItem->UnEquip();
	}
	if(IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopFire();
		CurrentRangeWeapon->EndReload(false);
		CurrentRangeWeapon->OnAmmoChanged.Remove(OnCurrentWeaponAmmoChangedHandle);
		CurrentRangeWeapon->OnReloadComplete.Remove(OnCurrentWeaponReloadedHandle);
	}
	if(!IgnoreSlotsWhileSwitching.Contains(CurrentEquippedSlot))
	{
		PreviousEquippedSlot = CurrentEquippedSlot;
	}
	CurrentEquippedSlot = EEquipmentSlots::None;
}

void UCharacterEquipmentComponent::ChangeCurrentWeaponRegime()
{
	if(IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->ChangeWeaponRegime();
		if(OnEquippedItemChanged.IsBound())
		{
			OnEquippedItemChanged.Broadcast(CurrentEquippedItem);
		}	
	}
}

void UCharacterEquipmentComponent::EquipNextItem()
{
	uint32 CurrentSlotIndex = (uint32)CurrentEquippedSlot;
	uint32 NextSlotIndex = NextItemsArraySlotIndex(CurrentSlotIndex);
	while(CurrentSlotIndex != NextSlotIndex && (!IsValid(ItemsArray[NextSlotIndex]) || IgnoreSlotsWhileSwitching.Contains((EEquipmentSlots)NextSlotIndex)))
	{
		NextSlotIndex = NextItemsArraySlotIndex(NextSlotIndex);
	}
	
	if(CurrentSlotIndex != NextSlotIndex)
	{
		EquipItemInSlot((EEquipmentSlots)NextSlotIndex);
	}
}

void UCharacterEquipmentComponent::EquipPreviousItem()
{
	uint32 CurrentSlotIndex = (uint32)CurrentEquippedSlot;
	uint32 PreviousSlotIndex = PreviousItemsArraySlotIndex(CurrentSlotIndex);
	while(CurrentSlotIndex != PreviousSlotIndex && (!IsValid(ItemsArray[PreviousSlotIndex]) || IgnoreSlotsWhileSwitching.Contains((EEquipmentSlots)PreviousSlotIndex)))
	{
		PreviousSlotIndex = PreviousItemsArraySlotIndex(PreviousSlotIndex);
	}
	if(CurrentSlotIndex != PreviousSlotIndex)
	{
		EquipItemInSlot((EEquipmentSlots)PreviousSlotIndex);
	}
}

void UCharacterEquipmentComponent::ReloadCurrentWeapon()
{
	check(IsValid(CurrentRangeWeapon));
	int32 AvailableAmunition = GetAvailableAmmunitionForCurrentWeapon();
	if(AvailableAmunition <= 0)
	{
		return;
	}

	CurrentRangeWeapon->Server_StartReload();
}

void UCharacterEquipmentComponent::ReloadAmmoInCurrentWeapon(int32 NumberOfAmmo, bool bCheckIsFull)
{
	int32 AvailableAmunition = GetAvailableAmmunitionForCurrentWeapon();
	
	int32 AmmoToReload = CurrentRangeWeapon->GetMaxAmmo() - CurrentRangeWeapon->GetAmmo();
	int32 ReloadedAmmo = FMath::Min(AvailableAmunition, AmmoToReload);

	if(NumberOfAmmo > 0)
	{
		ReloadedAmmo = FMath::Min(ReloadedAmmo, NumberOfAmmo);
	}
	
	AmmunitionArray[(uint32)CurrentRangeWeapon->GetAmmoType()] -= ReloadedAmmo;
	CurrentRangeWeapon->AddAmmo(ReloadedAmmo);

	if(bCheckIsFull)
	{
		AvailableAmunition = AmmunitionArray[(uint32)CurrentRangeWeapon->GetAmmoType()];
		bool bIsFullyReloaded = CurrentRangeWeapon->GetMaxAmmo() == CurrentRangeWeapon->GetAmmo();
		if(AvailableAmunition == 0 || bIsFullyReloaded)
		{
			CurrentRangeWeapon->EndReload(true);
		}
	}
	
}

bool UCharacterEquipmentComponent::AddEquipmentItemToSlot(const TSubclassOf<AEquipableItem> EquipableItemClass,
                                                          int32 SlotIndex)
{
	if(!IsValid(EquipableItemClass))
	{
		return false;
	}
	
	AEquipableItem* DefaultItemObject = EquipableItemClass->GetDefaultObject<AEquipableItem>();

	if(!DefaultItemObject->IsSlotCompatible(static_cast<EEquipmentSlots>(SlotIndex)))
	{
		return false;
	}

	if(!IsValid(ItemsArray[SlotIndex]))
	{
		AEquipableItem* Item = GetWorld()->SpawnActor<AEquipableItem>(EquipableItemClass);
		Item->AttachToComponent(CachedBaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, Item->GetUnEquippedSocketName());
		Item->SetOwner(CachedBaseCharacter.Get());
		Item->UnEquip();
		ItemsArray[SlotIndex] = Item;
	}
	else if (DefaultItemObject->IsA<ARangeWeaponItem>())
	{
		ARangeWeaponItem* RangeWeaponObject = StaticCast<ARangeWeaponItem*>(DefaultItemObject);
		int32 AmmoSlotIndex = static_cast<int32>(RangeWeaponObject->GetAmmoType());
		AmmunitionArray[SlotIndex] += RangeWeaponObject->GetMaxAmmo();
	}

	return true;
}

void UCharacterEquipmentComponent::RemoveItemFromSlot(int32 SlotIndex)
{
	if(static_cast<uint32>(CurrentEquippedSlot) == SlotIndex)
	{
		UnEquipCurrentItem();
	}
	ItemsArray[SlotIndex]->Destroy();
	ItemsArray[SlotIndex] = nullptr;
}

void UCharacterEquipmentComponent::OpenViewEquipment(APlayerController* PlayerController)
{
	if(!IsValid(ViewWidget))
	{
		CreateViewWidget(PlayerController);
	}

	if(!ViewWidget->IsVisible())
	{
		ViewWidget->AddToViewport();
	}
}

void UCharacterEquipmentComponent::CloseViewEquipment()
{
	if(ViewWidget->IsVisible())
	{
		ViewWidget->RemoveFromParent();
	}
}

bool UCharacterEquipmentComponent::IsViewVisible() const
{
	return ViewWidget->IsVisible();
}

const TArray<AEquipableItem*> UCharacterEquipmentComponent::GetItems() const
{
	return ItemsArray;
}

void UCharacterEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	checkf(GetOwner()->IsA<ABaseCharacter>(), TEXT("UCharacterEquipmentComponent::BeginPlay() CharacterEquipmentComponent can be used only with a BaseCharacter"));
	CachedBaseCharacter = StaticCast<ABaseCharacter*>(GetOwner());
	CreateLoadOut();
	AutoEquip();
}

void UCharacterEquipmentComponent::CreateViewWidget(APlayerController* PlayerController)
{
	check(IsValid(ViewWidgetClass));

	if(!IsValid(PlayerController))
	{
		return;
	}

	ViewWidget = CreateWidget<UEquipmentViewWidget>(PlayerController, ViewWidgetClass);
	ViewWidget->InitializeEquipmentWidget(this);
}

int32 UCharacterEquipmentComponent::GetAvailableAmmunitionForCurrentWeapon()
{
	check(IsValid(GetCurrentRangeWeapon()));
	return AmmunitionArray[(uint32)GetCurrentRangeWeapon()->GetAmmoType()];
}

void UCharacterEquipmentComponent::Server_EquipItemInSlot_Implementation(EEquipmentSlots Slot)
{
	EquipItemInSlot(Slot);
}

void UCharacterEquipmentComponent::OnRep_ItemsArray()
{
	for(auto Item : ItemsArray)
	{
		if(IsValid(Item))
		{
			Item->UnEquip();
		}
	}
}

void UCharacterEquipmentComponent::CreateLoadOut()
{
	if(GetOwnerRole() < ROLE_Authority)
	{
		return;
	}
	
	AmmunitionArray.AddZeroed((uint32)EAmunitionType::MAX);
	for( const TPair<EAmunitionType, int32>& AmmoPair : MaxAmmunitionAmount)
	{
		AmmunitionArray[(uint8)AmmoPair.Key] = FMath::Max(AmmoPair.Value, 0);
		if(AmmoPair.Key == EAmunitionType::FragGrenades)
		{
			OnCurrentThrowableItemAmountChanged(AmmunitionArray[(uint8)AmmoPair.Key]);
		}
	}

	ItemsArray.AddZeroed((uint32)EEquipmentSlots::MAX);
	for(const TPair<EEquipmentSlots, TSubclassOf<AEquipableItem>>& ItemPair : ItemsLoadOut)
	{
		if(!IsValid(ItemPair.Value))
		{
			continue;
		}
		
		// AddEquipmentItemToSlot(ItemPair.Value, static_cast<int32>(ItemPair.Key));
		AEquipableItem* Item = GetWorld()->SpawnActor<AEquipableItem>(ItemPair.Value);
		Item->AttachToComponent(CachedBaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, Item->GetUnEquippedSocketName());
		Item->SetOwner(CachedBaseCharacter.Get());
		Item->UnEquip();
		ItemsArray[(uint32)ItemPair.Key] = Item;

		//Creating loadout for grenades*
		//*or other throwable item
		if(ItemPair.Key == EEquipmentSlots::PrimaryItemSlot)
		{
			AThrowableItem* CurrentItem = Cast<AThrowableItem>(Item);
			CurrentThrowableItem = CurrentItem;
			CurrentThrowableItem->AddItems(AmmunitionArray[(uint8)CurrentThrowableItem->GetThrowableItemType()]);
			
			ThrowableItemPool.Reserve(ThrowableItemPoolSize);
			for(int32 i = 0; i < ThrowableItemPoolSize; i++)
			{
				AProjectile* ThrowableItem = GetWorld()->SpawnActor<AProjectile>(ThrowableItemClass, ThrowableItemPoolLocation, FRotator::ZeroRotator);
				if(!IsValid(ThrowableItem))
				{
					continue;
				}
				ThrowableItem->SetProjectileActive(false);
				ThrowableItemPool.Add(ThrowableItem);
			}
		}
	}
}

void UCharacterEquipmentComponent::AutoEquip()
{
	if(AutoEquipItemInSlot != EEquipmentSlots::None)
	{
		EquipItemInSlot(AutoEquipItemInSlot);
	}
}

uint32 UCharacterEquipmentComponent::NextItemsArraySlotIndex(uint32 CurrentSlotIndex)
{
	if(CurrentSlotIndex == ItemsArray.Num() - 1)
	{
		return 1;
	}
	else
	{
		return CurrentSlotIndex + 1;		
	}
}

uint32 UCharacterEquipmentComponent::PreviousItemsArraySlotIndex(uint32 CurrentSlotIndex)
{
	if(CurrentSlotIndex == 1 || CurrentSlotIndex == 0)
	{
		return ItemsArray.Num() - 1;
	}
	else
	{
		return CurrentSlotIndex - 1;		
	}
}

void UCharacterEquipmentComponent::OnWeaponReloadComplete()
{
	ReloadAmmoInCurrentWeapon();
}

void UCharacterEquipmentComponent::OnCurrentWeaponAmmoChanged(int32 Ammo)
{
	if(OnCurrentWeaponAmmoChangedEvent.IsBound())
	{
		OnCurrentWeaponAmmoChangedEvent.Broadcast(Ammo, GetAvailableAmmunitionForCurrentWeapon());
	}
}

void UCharacterEquipmentComponent::OnCurrentThrowableItemAmountChanged(int32 Amount)
{
	if(OnCurrentThrowableItemAmountChangedEvent.IsBound())
	{
		OnCurrentThrowableItemAmountChangedEvent.Broadcast(Amount);
	}
}

void UCharacterEquipmentComponent::OnRep_CurrentEquippedSLot(EEquipmentSlots CurrentEquippedSlot_Old)
{
	EquipItemInSlot(CurrentEquippedSlot);
}
