// Fill out your copyright notice in the Description page of Project Settings.


#include "FPPlayerAnimInstance.h"

#include "MyProject/Characters/FPPlayerCharacter.h"
#include "MyProject/Characters/Controllers/BaseCharacterPlayerController.h"

void UFPPlayerAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<AFPPlayerCharacter>(), TEXT("UFPPlayerAnimInstance::NativeBeginPlay() only FPPlayerCharacter can use UFPPlayerAnimInstance"));
	CachedFirstPersonCharacterOwner = StaticCast<AFPPlayerCharacter*>(TryGetPawnOwner());
}

void UFPPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if(!CachedFirstPersonCharacterOwner.IsValid())
	{
		return;
	}
	APlayerController* Controller = CachedFirstPersonCharacterOwner->GetController<APlayerController>();
	PlayerCameraPitchAngle = CalculateCameraPitchAngle();
	
}

float UFPPlayerAnimInstance::CalculateCameraPitchAngle() const
{
	float Result = 0.f;
	ABaseCharacterPlayerController* Controller = CachedFirstPersonCharacterOwner->GetController<ABaseCharacterPlayerController>();
	if(IsValid(Controller) && !Controller->GetIgnoreCameraPitch())
	{
		Result = Controller->GetControlRotation().Pitch;
	}
	
	return Result;
}
