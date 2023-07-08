// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"

#include "AttributesWidget.h"
#include "ReticleWidget.h"
#include "AmmoWidget.h"
#include "HighlightInteractable.h"
#include "Blueprint/WidgetTree.h"

UReticleWidget* UPlayerHUDWidget::GetReticleWidget()
{
	return WidgetTree->FindWidget<UReticleWidget>(ReticleWidgetName);
}

UAmmoWidget* UPlayerHUDWidget::GetAmmoWidget()
{
	return WidgetTree->FindWidget<UAmmoWidget>(AmmoWidgetName);
}

UAttributesWidget* UPlayerHUDWidget::GetAttributesWidget()
{
	return WidgetTree->FindWidget<UAttributesWidget>(AttributesWidgetName);
}

void UPlayerHUDWidget::SetHighlightInteractableVisibility(bool bIsVisible)
{
	if(!InteractableKey)
	{
		return;
	}
	
	if(bIsVisible)
	{
		InteractableKey->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		InteractableKey->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPlayerHUDWidget::SetHighlightInteractableActionText(FName KeyName)
{
	if(InteractableKey)
	{
		InteractableKey->SetActionText(KeyName);
	}
}
