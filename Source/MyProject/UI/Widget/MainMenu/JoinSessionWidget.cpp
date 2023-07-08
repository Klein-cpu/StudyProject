// Fill out your copyright notice in the Description page of Project Settings.


#include "JoinSessionWidget.h"

#include "BaseGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UJoinSessionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	check(GameInstance->IsA<UBaseGameInstance>());
	BaseGameInstance = StaticCast<UBaseGameInstance*>(GameInstance);
}

void UJoinSessionWidget::FindOnlineSession()
{
	BaseGameInstance->OnMatchFound.AddUFunction(this, FName("OnMatchFound"));
	BaseGameInstance->FindAMatch(bIsLAN);
	SearchingSessionState = ESearchingSessionState::Searching;
}

void UJoinSessionWidget::JoinOnlineSession()
{
	BaseGameInstance->JoinOnlineGame();
}

void UJoinSessionWidget::CloseWidget()
{
	Super::CloseWidget();
	BaseGameInstance->OnMatchFound.RemoveAll(this);
}

void UJoinSessionWidget::OnMatchFound_Implementation(bool bIsSuccessful)
{
	SearchingSessionState = bIsSuccessful ? ESearchingSessionState::SessionIsFound : ESearchingSessionState::None;
	BaseGameInstance->OnMatchFound.RemoveAll(this);
}
