// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameCodeTypes.h"
#include "CharacterEquipmentComponent.generated.h"

class UEquipmentViewWidget;
class AProjectile;
class AMeleeWeaponItem;
class AThrowableItem;

typedef TArray<int32, TInlineAllocator<static_cast<uint32>(EAmunitionType::MAX)>> TAmmunitionArray;
typedef TArray<class AEquipableItem*, TInlineAllocator<static_cast<uint32>(EEquipmentSlots::MAX)>> TItemsArray;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCurrentWeaponAmmoChangedEvent, int32, int32)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentThrowableItemAmountChangedEvent, int32)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquippedItemChanged, const AEquipableItem*)

class ARangeWeaponItem;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYPROJECT_API UCharacterEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UCharacterEquipmentComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	EEquipableItemType GetCurrentEquippedItemType() const;

	ARangeWeaponItem* GetCurrentRangeWeapon() const;

	AMeleeWeaponItem* GetCurrentMeleeWeapon() const;

	AThrowableItem* GetCurrentThrowableItem() const;
	
	bool IsEquipping() const;
	
	void AttachCurrentItemToEquippedSocket();

	void LaunchCurrentThrowableItem();

	void UnEquipCurrentItem();

	void ChangeCurrentWeaponRegime();
	
	FOnCurrentWeaponAmmoChangedEvent OnCurrentWeaponAmmoChangedEvent;
	FOnCurrentThrowableItemAmountChangedEvent OnCurrentThrowableItemAmountChangedEvent;
	FOnEquippedItemChanged OnEquippedItemChanged;
	
	void EquipItemInSlot(EEquipmentSlots Slot);

	void EquipNextItem();
	void EquipPreviousItem();
	
	void ReloadCurrentWeapon();

	void ReloadAmmoInCurrentWeapon(int32 NumberOfAmmo = 0, bool bCheckIsFull = false);

	bool AddEquipmentItemToSlot(const TSubclassOf<AEquipableItem> EquipableItemClass, int32 SlotIndex);
	void RemoveItemFromSlot(int32 SlotIndex);

	void OpenViewEquipment(APlayerController* PlayerController);
	void CloseViewEquipment();
	bool IsViewVisible() const;

	const TArray<AEquipableItem*> GetItems() const;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TMap<EAmunitionType, int32> MaxAmmunitionAmount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TMap<EEquipmentSlots, TSubclassOf<AEquipableItem>> ItemsLoadOut;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TSet<EEquipmentSlots> IgnoreSlotsWhileSwitching;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	EEquipmentSlots AutoEquipItemInSlot = EEquipmentSlots::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "View")
	TSubclassOf<UEquipmentViewWidget> ViewWidgetClass;

	void CreateViewWidget(APlayerController* PlayerController);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout | Throwable items")
	TSubclassOf<AProjectile> ThrowableItemClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout | Throwable items", meta = (UIMin = 1, ClampMin = 1))
	int32 ThrowableItemPoolSize = 5;
	
private:
	UFUNCTION(Server, Reliable)
	void Server_EquipItemInSlot(EEquipmentSlots Slot);

	UPROPERTY(Replicated)
	TArray<int32> AmmunitionArray;

	UPROPERTY(ReplicatedUsing = OnRep_ItemsArray)
	TArray<AEquipableItem*> ItemsArray;

	UFUNCTION()
	void OnRep_ItemsArray();
	
	UPROPERTY(Replicated)
	TArray<AProjectile*> ThrowableItemPool;

	UPROPERTY(Replicated)
	int32 CurrentThrowableItemIndex = 0;
	
	void CreateLoadOut();

	void AutoEquip();
	
	void EquipAnimationFinished();
	
	uint32 NextItemsArraySlotIndex(uint32 CurrentSlotIndex);
	uint32 PreviousItemsArraySlotIndex(uint32 CurrentSlotIndex);
	
	int32 GetAvailableAmmunitionForCurrentWeapon();

	bool bIsEquipping = false;
	
	UFUNCTION()
	void OnWeaponReloadComplete();
	
	UFUNCTION()
	void OnCurrentWeaponAmmoChanged(int32 Ammo);

	UFUNCTION()
	void OnCurrentThrowableItemAmountChanged(int32 Amount);
	
	const FVector ThrowableItemPoolLocation = FVector(0.f,2.f,-100.f);
	
	FDelegateHandle OnCurrentWeaponAmmoChangedHandle;
	FDelegateHandle OnCurrentWeaponReloadedHandle;

	EEquipmentSlots PreviousEquippedSlot;
	UPROPERTY(ReplicatedUsing=OnRep_CurrentEquippedSlot)
	EEquipmentSlots CurrentEquippedSlot;

	UFUNCTION()
	void OnRep_CurrentEquippedSLot(EEquipmentSlots CurrentEquippedSlot_Old);
	
	UPROPERTY()
	AEquipableItem* CurrentEquippedItem;
	UPROPERTY()
	ARangeWeaponItem* CurrentRangeWeapon;
	UPROPERTY()
	AThrowableItem* CurrentThrowableItem;
	UPROPERTY()
	AMeleeWeaponItem* CurrentMeleeWeapon;

	TWeakObjectPtr<class ABaseCharacter> CachedBaseCharacter;

	FTimerHandle EquipTimer;

	UEquipmentViewWidget* ViewWidget;
	
};