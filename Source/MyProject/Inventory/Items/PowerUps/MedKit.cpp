// Fill out your copyright notice in the Description page of Project Settings.


#include "MedKit.h"

#include "Characters/BaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"

bool UMedKit::Consume(ABaseCharacter* ConsumeTarget)
{
	UCharacterAttributesComponent* CharacterAttributes = ConsumeTarget->GetCharacterAttributesComponent();
	CharacterAttributes->AddHealth(Health);
	this->ConditionalBeginDestroy();
	return true;
}
