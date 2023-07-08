// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentSlotWidget.h"

#include "Actors/Equipment/EquipableItem.h"
#include "Actors/Interactive/Pickables/PickableItem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Characters/BaseCharacter.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Inventory/Items/InventoryItem.h"
#include "Inventory/Items/Equipables/WeaponInventoryItem.h"
#include "UI/Widget/Inventory/InventorySlotWidget.h"
#include "Utils/BaseDataTableUtils.h"

void UEquipmentSlotWidget::InitializeEquipmentSlot(TWeakObjectPtr<AEquipableItem> Equipment, int32 Index)
{
	if(!Equipment.IsValid())
	{
		return;
	}

	LinkedEquipableItem = Equipment;
	SlotIndexInComponent = Index;

	FWeaponTableRow* EquipmentData = BaseDataTableUtils::FindWeaponData(Equipment->GetDataTableID());
	if(EquipmentData != nullptr)
	{
		AdapterLinkedInventoryItem = NewObject<UWeaponInventoryItem>(Equipment->GetOwner());
		AdapterLinkedInventoryItem->Initialize(Equipment->GetDataTableID(), EquipmentData->WeaponItemDescription);
		AdapterLinkedInventoryItem->SetEquipWeaponClass(EquipmentData->EquipableActor);
	}
}

void UEquipmentSlotWidget::UpdateView()
{
	if(LinkedEquipableItem.IsValid())
	{
		ImageWeaponIcon->SetBrushFromTexture(AdapterLinkedInventoryItem->GetDescription().Icon);
		TBWeaponName->SetText(AdapterLinkedInventoryItem->GetDescription().Name);
	}
	else
	{
		ImageWeaponIcon->SetBrushFromTexture(nullptr);
		TBWeaponName->SetText(FText::FromName(NAME_None));
	}
}

FReply UEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if(!LinkedEquipableItem.IsValid())
	{
		return FReply::Handled();
	}

	FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
	return Reply.NativeReply;
}

void UEquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	checkf(DragDropWidgetClass.Get() != nullptr, TEXT("UEquipmentSlotWidget::NativeOnDragDetected drag and drop widget is not defined"));

	if(!AdapterLinkedInventoryItem.IsValid())
	{
		return;
	}

	UDragDropOperation* DragOperation = Cast<UDragDropOperation>(UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass()));

	UInventorySlotWidget* DragWidget = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), DragDropWidgetClass);
	DragWidget->SetItemIcon(AdapterLinkedInventoryItem->GetDescription().Icon);

	DragOperation->DefaultDragVisual = DragWidget;
	DragOperation->Pivot = EDragPivot::CenterCenter;
	DragOperation->Payload = AdapterLinkedInventoryItem.Get();
	OutOperation = DragOperation;

	LinkedEquipableItem.Reset();
	OnEquipmentRemoveFromSlot.ExecuteIfBound(SlotIndexInComponent);

	UpdateView();
}

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const UWeaponInventoryItem* OperationObject = Cast<UWeaponInventoryItem>(InOperation->Payload);
	if(IsValid(OperationObject))
	{
		return OnEquipmentDropInSlot.Execute(OperationObject->GetEquipWeaponClass(), SlotIndexInComponent);
	}
	return false;
}

void UEquipmentSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UInventoryItem* CurrentOperatedItem = Cast<UWeaponInventoryItem>(InOperation->Payload);
	
	TSubclassOf<APickableItem> ItemToDropClass;
	ItemToDropClass = BaseDataTableUtils::FindWeaponData(CurrentOperatedItem->GetDataTableID())->PickableActor;

	ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwningPlayer()->GetCharacter());
	if(!IsValid(Character))
	{
		return;
	}
	
	FVector ItemToDropLocation = Character->GetActorLocation() + 50.f * Character->GetActorForwardVector();
	
	//A little bit of hard coding just because this value is comfortable for me. Can be changed, but I don' really wanna do this right know
	ItemToDropLocation.Z = 130.f;
	GetWorld()->SpawnActor<APickableItem>(ItemToDropClass, ItemToDropLocation, Character->GetActorForwardVector().ToOrientationRotator());
}
