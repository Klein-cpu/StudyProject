// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "FPPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API AFPPlayerCharacter : public APlayerCharacter
{
	GENERATED_BODY()

public:
	AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaSeconds) override;

	virtual void PossessedBy(AController* NewController) override;
	
	//Crouch
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual FRotator GetViewRotation() const override;
	
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	virtual void Falling() override;
	
	virtual void Landed(const FHitResult& Hit) override;
	
protected:
	//Basic components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | First person")
	class USkeletalMeshComponent* FirstPersonMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | First person")
	class UCameraComponent* FirstPersonCameraComponent;

	//Mantle
	virtual void OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime) override;

	//Ladders
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ladders | Pitch", meta = (UIMine = -89.f, UIMax = 89.f))
	float LadderCameraMinPitch = -60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ladders | Pitch", meta = (UIMine = -89.f, UIMax = 89.f))
	float LadderCameraMaxPitch = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ladders | Yaw", meta = (UIMine = 0.f, UIMax = 359.f))
	float LadderCameraMinYaw = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ladders | Yaw", meta = (UIMine = 0.f, UIMax = 359.f))
	float LadderCameraMaxYaw = 175.f;

	//Ziplines
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ziplines | Pitch", meta = (UIMine = -89.f, UIMax = 89.f))
	float ZiplineCameraMinPitch = -89.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ziplines | Pitch", meta = (UIMine = -89.f, UIMax = 89.f))
	float ZiplineCameraMaxPitch = 89.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ziplines | Yaw", meta = (UIMine = 0.f, UIMax = 359.f))
	float ZiplineCameraMinYaw = -90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ziplines | Yaw", meta = (UIMine = 0.f, UIMax = 359.f))
	float ZiplineCameraMaxYaw = 90.f;
	
private:
	FTimerHandle FPMontageTimer;

	void OnFPMontageTimerElapsed();
	bool IsFPMontagePlaying() const;

	void OnLadderStarted();
	void OnLadderStopped();

	void OnZiplineStarted();
	void OnZiplineStopped();

	TWeakObjectPtr<class ABaseCharacterPlayerController> BasePlayerController;
};
