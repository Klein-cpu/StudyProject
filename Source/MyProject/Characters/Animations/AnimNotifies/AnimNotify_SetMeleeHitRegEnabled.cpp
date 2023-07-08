// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animations/AnimNotifies/AnimNotify_SetMeleeHitRegEnabled.h"

#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "Characters/BaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void UAnimNotify_SetMeleeHitRegEnabled::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	ABaseCharacter* CharacterOwner = Cast<ABaseCharacter>(MeshComp->GetOwner());

	if(!IsValid(CharacterOwner))
	{
		return;
	}
	AMeleeWeaponItem* MeleeWeapon = CharacterOwner->GetCharacterEquipmentComponent()->GetCurrentMeleeWeapon();
	if(IsValid(MeleeWeapon))
	{
		MeleeWeapon->SetIsHitRegistrationEnabled(bIsHitRegistrationEnabled);
	}
}
