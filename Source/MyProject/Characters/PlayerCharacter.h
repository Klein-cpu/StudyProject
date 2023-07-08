// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "Components/TimelineComponent.h"
#include "PlayerCharacter.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MYPROJECT_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual void MoveForward(float Value) override;
	virtual void MoveRight(float Value) override;
	virtual void Turn(float Value) override;
	virtual void LookUp(float Value) override;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	
	virtual void OnStartSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	
	virtual void SwimForward(float Value) override;
	virtual void SwimRight(float Value) override;
	virtual void SwimUp(float Value) override;
	
	virtual void OnJumped_Implementation() override;
	
	virtual void OnSprintStart_Implementation() override;
	virtual void OnSprintEnd_Implementation() override;

	virtual void Tick(float DeltaSeconds) override;

	
protected:

	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Character | Camera")
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Character | Camera")
	class USpringArmComponent* SpringArmComponent;
	
	UPROPERTY(EditAnywhere, Category = "Character | Sprint")
	UCurveFloat* SprintTimelineCurve;

	UPROPERTY(EditAnywhere, Category = "Character | Aim")
	UCurveFloat* AimTimelineCurve;

	UFUNCTION()
	void SprintTimelineUpdate(float Alpha) const;

	UFUNCTION()
	void AimTimelineUpdate(float Alpha) const;

	UPROPERTY()
	FTimeline SprintTimeline;

	UPROPERTY()
	FTimeline AimTimeline;
	
	UPROPERTY()
	float DefaultArmLength = 300.f;

	UPROPERTY(EditAnywhere, Category = "Character | Sprint")
	float SprintArmLength = 600.f;

	virtual void OnStartAimingInternal() override;
	
	virtual void OnStopAimingInternal() override;

	mutable float AimTurnModifier;
	mutable float AimLookUpModifier;
	
};
