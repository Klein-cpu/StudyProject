// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeProgressBar.h"

#include "Components/ProgressBar.h"

void UAttributeProgressBar::SetProgressPercentage(float Percentage)
{
	HealthProgressBar->SetPercent(Percentage);
}
