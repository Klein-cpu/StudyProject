// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/AttributesWidget.h"

#include "Characters/BaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"

float UAttributesWidget::GetHealthPercent() const
{
	float Result = 1.f;
	
	APawn* Pawn = GetOwningPlayerPawn();
	ABaseCharacter* Character = Cast<ABaseCharacter>(Pawn);
	if(IsValid(Character))
	{
		const UCharacterAttributesComponent* CharacterAttributes = Character->GetCharacterAttributesComponent();
		Result = CharacterAttributes->GetHealthPercent();
	}
	
	return Result;
}

float UAttributesWidget::GetStaminaPercent() const
{
	float Result = 1.f;
	
	APawn* Pawn = GetOwningPlayerPawn();
	ABaseCharacter* Character = Cast<ABaseCharacter>(Pawn);
	if(IsValid(Character))
	{
		const UCharacterAttributesComponent* CharacterAttributes = Character->GetCharacterAttributesComponent();
		Result = CharacterAttributes->GetStaminaPercent();
	}
	
	return Result;
}

float UAttributesWidget::GetOxygenPercent() const
{
	float Result = 1.f;
	
	APawn* Pawn = GetOwningPlayerPawn();
	ABaseCharacter* Character = Cast<ABaseCharacter>(Pawn);
	if(IsValid(Character))
	{
		const UCharacterAttributesComponent* CharacterAttributes = Character->GetCharacterAttributesComponent();
		Result = CharacterAttributes->GetOxygenPercent();
	}
	
	return Result;
}

ESlateVisibility UAttributesWidget::GetStaminaVisibility() const
{
	ESlateVisibility Result = ESlateVisibility::Hidden;

	APawn* Pawn = GetOwningPlayerPawn();
	ABaseCharacter* Character = Cast<ABaseCharacter>(Pawn);
	if(IsValid(Character))
	{
		const UCharacterAttributesComponent* CharacterAttributes = Character->GetCharacterAttributesComponent();
		if(CharacterAttributes->GetStaminaPercent() < 1.f)
		{
			Result = ESlateVisibility::Visible;
		}
	}

	return Result;
}

ESlateVisibility UAttributesWidget::GetOxygenVisibility() const
{
	ESlateVisibility Result = ESlateVisibility::Hidden;

	APawn* Pawn = GetOwningPlayerPawn();
	ABaseCharacter* Character = Cast<ABaseCharacter>(Pawn);
	if(IsValid(Character))
	{
		const UCharacterAttributesComponent* CharacterAttributes = Character->GetCharacterAttributesComponent();
		if(CharacterAttributes->GetOxygenPercent() < 1.f)
		{
			Result = ESlateVisibility::Visible;
		}
	}

	return Result;
}
