// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_AttachEquippedItem.h"

#include "Characters/BaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void UAnimNotify_AttachEquippedItem::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	ABaseCharacter* CharacterOwner = Cast<ABaseCharacter>(MeshComp->GetOwner());

	if(!IsValid(CharacterOwner))
	{
		return;
	}
	CharacterOwner->GetCharacterEquipmentComponent_Mutable()->AttachCurrentItemToEquippedSocket();
}

