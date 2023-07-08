// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeProgressBar.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class MYPROJECT_API UAttributeProgressBar : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetProgressPercentage(float Percentage);
	
protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthProgressBar;
};
