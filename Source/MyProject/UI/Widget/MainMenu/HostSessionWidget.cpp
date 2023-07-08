// Fill out your copyright notice in the Description page of Project Settings.


#include "HostSessionWidget.h"

#include "BaseGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UHostSessionWidget::CreateSession()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	check(GameInstance->IsA<UBaseGameInstance>());
	UBaseGameInstance* BaseGameInstance = StaticCast<UBaseGameInstance*>(GameInstance);

	BaseGameInstance->LaunchLobby(4, ServerName, bIsLAN);
}
