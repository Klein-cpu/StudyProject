// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Items/PowerUps/Adrenaline.h"

#include "Characters/BaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"

bool UAdrenaline::Consume(ABaseCharacter* ConsumeTarget)
{
	UCharacterAttributesComponent* CharacterAttributes = ConsumeTarget->GetCharacterAttributesComponent();
	CharacterAttributes->AddStamina();
	this->ConditionalBeginDestroy();
	return true;
}
