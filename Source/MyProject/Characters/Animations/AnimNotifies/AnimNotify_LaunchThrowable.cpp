// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animations/AnimNotifies/AnimNotify_LaunchThrowable.h"

#include "Characters/BaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void UAnimNotify_LaunchThrowable::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	ABaseCharacter* CharacterOwner = Cast<ABaseCharacter>(MeshComp->GetOwner());
	if(!IsValid(CharacterOwner))
	{
		return;
	}
	CharacterOwner->GetCharacterEquipmentComponent_Mutable()->LaunchCurrentThrowableItem();
}
