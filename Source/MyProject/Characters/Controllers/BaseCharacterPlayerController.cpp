// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacterPlayerController.h"

#include "GameCodeTypes.h"
#include "Blueprint/UserWidget.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "MyProject/Characters/BaseCharacter.h"
#include "UI/Widget/AmmoWidget.h"
#include "UI/Widget/PlayerHUDWidget.h"
#include "UI/Widget/ReticleWidget.h"

void ABaseCharacterPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	CachedBaseCharacter = Cast<ABaseCharacter>(InPawn);
	if(CachedBaseCharacter.IsValid() && IsLocalController())
	{
		CreateAndInitializeWidgets();
		CachedBaseCharacter->OnInteractableObjectFound.BindUObject(this, &ABaseCharacterPlayerController::OnInteractableObjectFound);
	}
}

bool ABaseCharacterPlayerController::GetIgnoreCameraPitch() const
{
	return bIgnoreCameraPitch;
}

void ABaseCharacterPlayerController::SetIgnoreCameraPitch(bool bIgnoreCameraPitch_In)
{
	bIgnoreCameraPitch = bIgnoreCameraPitch_In;
}

void ABaseCharacterPlayerController::MoveForward(float Value)
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->MoveForward(Value);
	}
}

void ABaseCharacterPlayerController::MoveRight(float Value)
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->MoveRight(Value);
	}
}

void ABaseCharacterPlayerController::Turn(float Value)
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Turn(Value);
	}
}

void ABaseCharacterPlayerController::LookUp(float Value)
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->LookUp(Value);
	}
}

void ABaseCharacterPlayerController::SwimForward(float Value)
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimForward(Value);
	}
}

void ABaseCharacterPlayerController::SwimRight(float Value)
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimRight(Value);
	}
	
}

void ABaseCharacterPlayerController::SwimUp(float Value)
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimUp(Value);
	}
	
}

void ABaseCharacterPlayerController::ClimbLadderUp(float Value)
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ClimbLadderUp(Value);
	}
}

void ABaseCharacterPlayerController::Mantle()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Mantle();
	}
}

void ABaseCharacterPlayerController::Jump()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Jump();
	}
}

void ABaseCharacterPlayerController::StandUp()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StandUp();
	}
}

void ABaseCharacterPlayerController::ChangeCrouchState()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ChangeCrouchState();
	}
}

void ABaseCharacterPlayerController::StartSprint()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartSprint();
	}
}

void ABaseCharacterPlayerController::StopSprint()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopSprint();
	}
}

void ABaseCharacterPlayerController::ChangeProneState()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ChangeProneState();
	}
}

void ABaseCharacterPlayerController::InteractWithLadder()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->InteractWithLadder();
	}
}

void ABaseCharacterPlayerController::AttachToZipline()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->AttachToZipline();
	}
}

void ABaseCharacterPlayerController::Slide()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Slide();
	}
}

void ABaseCharacterPlayerController::BeginFire()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartFire();
	}
}

void ABaseCharacterPlayerController::StopFire()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopFire();
	}
}

void ABaseCharacterPlayerController::StartAiming()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartAiming();
	}
}

void ABaseCharacterPlayerController::StopAiming()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopAiming();
	}
}

void ABaseCharacterPlayerController::Reload()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Reload();
	}
}

void ABaseCharacterPlayerController::NextItem()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->NextItem();
	}
}

void ABaseCharacterPlayerController::PreviousItem()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->PreviousItem();
	}
}

void ABaseCharacterPlayerController::EquipPrimaryItem()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->EquipPrimaryItem();
	}
}

void ABaseCharacterPlayerController::PrimaryMeleeAttack()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->PrimaryMeleeAttack();
	}
}

void ABaseCharacterPlayerController::SecondaryMeleeAttack()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SecondaryMeleeAttack();
	}
}

void ABaseCharacterPlayerController::ChangeWeaponRegime()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ChangeWeaponRegime();
	}
}

void ABaseCharacterPlayerController::Interact()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Interact();
	}
}

void ABaseCharacterPlayerController::UseInventory()
{
	if(CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->UseInventory(this);
	}
}

void ABaseCharacterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	InputComponent->BindAxis("MoveForward", this, &ABaseCharacterPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ABaseCharacterPlayerController::MoveRight);
	InputComponent->BindAxis("Turn", this, &ABaseCharacterPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &ABaseCharacterPlayerController::LookUp);
	InputComponent->BindAxis("SwimForward", this, &ABaseCharacterPlayerController::SwimForward);
	InputComponent->BindAxis("SwimRight", this, &ABaseCharacterPlayerController::SwimRight);
	InputComponent->BindAxis("SwimUp", this, &ABaseCharacterPlayerController::SwimUp);
	InputComponent->BindAxis("ClimbLadderUp", this, &ABaseCharacterPlayerController::ClimbLadderUp);

	InputComponent->BindAction("AttachToZipline", IE_Pressed, this, &ABaseCharacterPlayerController::AttachToZipline);
	InputComponent->BindAction("InteractWithLadder", IE_Pressed, this, &ABaseCharacterPlayerController::InteractWithLadder);
	InputComponent->BindAction("Mantle", IE_Pressed, this, &ABaseCharacterPlayerController::Mantle);
	InputComponent->BindAction("Jump", IE_Pressed, this, &ABaseCharacterPlayerController::Jump);
	InputComponent->BindAction("StandUp", IE_Pressed, this, &ABaseCharacterPlayerController::StandUp);
	InputComponent->BindAction("Slide", IE_Pressed, this, &ABaseCharacterPlayerController::Slide);
	InputComponent->BindAction("Crouch", IE_Pressed, this, &ABaseCharacterPlayerController::ChangeCrouchState);
	InputComponent->BindAction("Sprint", IE_Pressed, this, &ABaseCharacterPlayerController::StartSprint);
	InputComponent->BindAction("Sprint", IE_Released, this, &ABaseCharacterPlayerController::StopSprint);
	InputComponent->BindAction("Prone", IE_Pressed, this, &ABaseCharacterPlayerController::ChangeProneState);
	InputComponent->BindAction("Fire", IE_Pressed, this, &ABaseCharacterPlayerController::BeginFire);
	InputComponent->BindAction("Fire", IE_Released, this, &ABaseCharacterPlayerController::StopFire);
	InputComponent->BindAction("Aim", IE_Pressed, this, &ABaseCharacterPlayerController::StartAiming);
	InputComponent->BindAction("Aim", IE_Released, this, &ABaseCharacterPlayerController::StopAiming);
	InputComponent->BindAction("Reload", IE_Pressed, this, &ABaseCharacterPlayerController::Reload);
	InputComponent->BindAction("EquipNextItem", IE_Pressed, this, &ABaseCharacterPlayerController::NextItem);
	InputComponent->BindAction("EquipPreviousItem", IE_Pressed, this, &ABaseCharacterPlayerController::PreviousItem);
	InputComponent->BindAction("EquipPrimaryItem", IE_Pressed, this, &ABaseCharacterPlayerController::EquipPrimaryItem);
	InputComponent->BindAction("PrimaryMeleeAttack", IE_Pressed, this, &ABaseCharacterPlayerController::PrimaryMeleeAttack);
	InputComponent->BindAction("SecondaryMeleeAttack", IE_Pressed, this, &ABaseCharacterPlayerController::SecondaryMeleeAttack);
	InputComponent->BindAction("ChangeWeaponRegime", IE_Pressed, this, &ABaseCharacterPlayerController::ChangeWeaponRegime);
	InputComponent->BindAction(ActionInteract, IE_Pressed, this, &ABaseCharacterPlayerController::Interact);
	FInputActionBinding& ToggleMenuBinding = InputComponent->BindAction("ToggleMainMenu", IE_Pressed, this, &ABaseCharacterPlayerController::ToggleMainMenu);
	ToggleMenuBinding.bExecuteWhenPaused = true;
	InputComponent->BindAction("UseInventory", IE_Pressed, this, &ABaseCharacterPlayerController::UseInventory);
}

void ABaseCharacterPlayerController::OnInteractableObjectFound(FName ActionName)
{
	if(!PlayerHUDWidget)
	{
		return;
	}

	TArray<FInputActionKeyMapping> ActionKeys = PlayerInput->GetKeysForAction(ActionName);
	const bool HasAnyKeys = ActionKeys.Num() != 0;
	if(HasAnyKeys)
	{
		FName ActionKey = ActionKeys[0].Key.GetFName();
		PlayerHUDWidget->SetHighlightInteractableActionText(ActionKey);
	}
	PlayerHUDWidget->SetHighlightInteractableVisibility(HasAnyKeys);
}

void ABaseCharacterPlayerController::CreateAndInitializeWidgets()
{
	if(!IsValid(PlayerHUDWidget))
	{
		PlayerHUDWidget = CreateWidget<UPlayerHUDWidget>(GetWorld(), PlayerHUDWidgetClass);
		if(IsValid(PlayerHUDWidget))
		{
			PlayerHUDWidget->AddToViewport();
		}
	}

	if(!IsValid(MainMenuWidget))
	{
		MainMenuWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
	}
	
	if(IsValid(PlayerHUDWidget) && CachedBaseCharacter.IsValid())
	{
		UReticleWidget* ReticleWidget = PlayerHUDWidget->GetReticleWidget();
		if(IsValid(ReticleWidget))
		{
			CachedBaseCharacter->OnAimingStateChangedEvent.AddUFunction(ReticleWidget, FName("OnAimingStateChanged"));
			UCharacterEquipmentComponent* CharacterEquipment = CachedBaseCharacter->GetCharacterEquipmentComponent_Mutable();
			CharacterEquipment->OnEquippedItemChanged.AddUFunction(ReticleWidget, FName("OnEquippedItemChanged"));
		}
		UAmmoWidget* AmmoWidget = PlayerHUDWidget->GetAmmoWidget();
		if(IsValid(AmmoWidget))
		{
			UCharacterEquipmentComponent* CharacterEquipment = CachedBaseCharacter->GetCharacterEquipmentComponent_Mutable();
			CharacterEquipment->OnCurrentWeaponAmmoChangedEvent.AddUFunction(AmmoWidget, FName("UpdateAmmoCount"));
			CharacterEquipment->OnCurrentThrowableItemAmountChangedEvent.AddUFunction(AmmoWidget, FName("UpdateGrenadesCount"));
		}
	}
	
	SetInputMode(FInputModeGameOnly{});
	bShowMouseCursor = false;
	
}

void ABaseCharacterPlayerController::ToggleMainMenu()
{
	if(!IsValid(MainMenuWidget) || !IsValid(PlayerHUDWidget))
	{
		return;
	}

	if(MainMenuWidget->IsVisible())
	{
		MainMenuWidget->RemoveFromParent();
		PlayerHUDWidget->AddToViewport();
		SetInputMode(FInputModeGameOnly{});
		SetPause(false);
		bShowMouseCursor = false;
	}
	else
	{
		MainMenuWidget->AddToViewport();
		PlayerHUDWidget->RemoveFromParent();
		SetInputMode(FInputModeGameAndUI{});
		SetPause(true);
		bShowMouseCursor = true;
	}
}
