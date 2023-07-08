// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCodeTypes.h"
#include "Animation/AnimInstance.h"
#include "MyProject/Characters/PlayerCharacter.h"
#include "MyProject/Components/MovementComponents/BaseCharacterMovementComponent.h"
#include "BaseCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API UBaseCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation", meta = (UIMin = 0.f, UIMax = 500.f))
	float Speed = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsFalling = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsProning = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsCrouching = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsSprinting = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsOutOfStamina = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsSwimming = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsOnLadder = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsOnZipline = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsWallRunning = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsSliding = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	bool bIsStrafing = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation | Weapon")
	bool bIsAiming = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation", meta = (UIMin = -180.f, UIMax = 180.f))
	float Direction = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	float LadderSpeedRatio = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	FRotator AimRotation = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation")
	EWallRunSide WallRunSide = EWallRunSide::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation | Weapon", meta = (UIMin = -180.f, UIMax = 180.f))
	EEquipableItemType CurrentEquippedItemType = EEquipableItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character animation | Weapon")
	FTransform ForeGripSocketTransform;
	
	UPROPERTY(EditAnywhere, Transient, BlueprintReadOnly, Category="Character | IK settings")
	FVector RightFootEffectorLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Transient, BlueprintReadOnly, Category="Character | IK settings")
	FVector LeftFootEffectorLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Transient, BlueprintReadOnly, Category="Character | IK settings")
	FVector PelvisEffectorLocation = FVector::ZeroVector;
	
private:
	
	TWeakObjectPtr<ABaseCharacter> CachedBaseCharacter;
	
};
