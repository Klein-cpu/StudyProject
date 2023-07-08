// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacterAnimInstance.h"
#include "FPPlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API UFPPlayerAnimInstance : public UBaseCharacterAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animations | FirstPerson")
	float PlayerCameraPitchAngle = 0.f;

	TWeakObjectPtr<class AFPPlayerCharacter> CachedFirstPersonCharacterOwner;

private:
	float CalculateCameraPitchAngle() const;
	
};
