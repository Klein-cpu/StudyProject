// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBasePawnAnimInstance.h"

#include "MyProject/Characters/GameCodeBasePawn.h"
#include "../../Components/MovementComponents/GCBasePawnMovementComponent.h"

void UGCBasePawnAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<AGameCodeBasePawn>(), TEXT("UGCBasePawnAnimInstance::NativeBeginPlay() only GameCodeBasePawn can work with UGCBasePawnAnimInstance"));
	CachedBasePawn = StaticCast<AGameCodeBasePawn*>(TryGetPawnOwner());
}

void UGCBasePawnAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if(!CachedBasePawn.IsValid())
	{
		return;
	}

	InputForward = CachedBasePawn->GetInputForward();
	InputRight = CachedBasePawn->GetInputRight();

	bIsInAir = CachedBasePawn->GetMovementComponent()->IsFalling();
	
}
