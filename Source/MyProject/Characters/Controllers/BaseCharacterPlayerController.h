// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BaseCharacterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API ABaseCharacterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void SetPawn(APawn* InPawn) override;

	bool GetIgnoreCameraPitch() const;

	void SetIgnoreCameraPitch(bool bIgnoreCameraPitch_In);
	
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void SwimForward(float Value);
	void SwimRight(float Value);
	void SwimUp(float Value);
	void ClimbLadderUp(float Value);
	
	void Mantle();
	void Jump();
	void StandUp();
	void ChangeCrouchState();
	void StartSprint();
	void StopSprint();
	void ChangeProneState();
	void InteractWithLadder();
	void AttachToZipline();
	void Slide();
	void BeginFire();
	void StopFire();
	void StartAiming();
	void StopAiming();
	void Reload();
	void NextItem();
	void PreviousItem();
	void EquipPrimaryItem();
	void PrimaryMeleeAttack();
	void SecondaryMeleeAttack();
	void ChangeWeaponRegime();
	void Interact();
	void UseInventory();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<class UPlayerHUDWidget> PlayerHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	
	virtual void SetupInputComponent() override;
	
private:
	void OnInteractableObjectFound(FName ActionName);
	
	void CreateAndInitializeWidgets();
	
	UPlayerHUDWidget* PlayerHUDWidget = nullptr;
	UUserWidget* MainMenuWidget = nullptr;
	
	TSoftObjectPtr<class ABaseCharacter> CachedBaseCharacter;

	bool bIgnoreCameraPitch = false;

	void ToggleMainMenu();
};
