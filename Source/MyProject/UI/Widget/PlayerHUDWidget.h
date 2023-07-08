// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UHighlightInteractable;
class UAttributesWidget;
class UAmmoWidget;
class UReticleWidget;
/**
 * 
 */
UCLASS()
class MYPROJECT_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UReticleWidget* GetReticleWidget();

	UAmmoWidget* GetAmmoWidget();

	UAttributesWidget* GetAttributesWidget();

	void SetHighlightInteractableVisibility(bool bIsVisible);

	void SetHighlightInteractableActionText(FName KeyName);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget names")
	FName ReticleWidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget names")
	FName AmmoWidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget names")
	FName AttributesWidgetName;

	UPROPERTY(meta = (BindWidget))
	UHighlightInteractable* InteractableKey;
	
};
