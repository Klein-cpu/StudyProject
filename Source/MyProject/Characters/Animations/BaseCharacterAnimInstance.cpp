// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacterAnimInstance.h"

#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "MyProject/Characters/BaseCharacter.h"
#include "MyProject/Components/MovementComponents/BaseCharacterMovementComponent.h"

void UBaseCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<ABaseCharacter>(), TEXT("UBaseCharacterAnimInstance::NativeBeginPlay() "
													   "UBaseCharacterAnimInstance can be used only with ABaseCharacter"));
	CachedBaseCharacter = StaticCast<ABaseCharacter*>(TryGetPawnOwner());
}

void UBaseCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if(!CachedBaseCharacter.IsValid())
	{
		return;
	}
	
	UBaseCharacterMovementComponent* CharacterMovement = CachedBaseCharacter->GetBaseCharacterMovementComponent();
	Speed = CharacterMovement->Velocity.Size();
	bIsFalling = CharacterMovement->IsFalling();
	bIsProning = CharacterMovement->IsProning();
	bIsSwimming = CharacterMovement->IsSwimming();
	bIsCrouching = CharacterMovement->IsCrouching();
	bIsSprinting = CharacterMovement->IsSprinting();
	bIsOutOfStamina = CharacterMovement->IsOutOfStamina();
	bIsOnLadder = CharacterMovement->IsOnLadder();
	bIsOnZipline = CharacterMovement->IsOnZipline();
	bIsAiming = CachedBaseCharacter->IsAiming();
	if(bIsOnLadder)
	{
		LadderSpeedRatio = CharacterMovement->GetLadderSpeedRatio();
	}
	bIsWallRunning = CharacterMovement->IsWallRunning();
	WallRunSide = CharacterMovement->GetWallRunSide();
	bIsStrafing = !CharacterMovement->bOrientRotationToMovement;
	Direction = CalculateDirection(CharacterMovement->Velocity, CachedBaseCharacter->GetActorRotation());
	
	RightFootEffectorLocation = FVector(CachedBaseCharacter->GetIKRightFootOffset() + CachedBaseCharacter->GetIKPelvisOffset() , 0.f, 0.f);
	LeftFootEffectorLocation = FVector(-(CachedBaseCharacter->GetIKLeftFootOffset() + CachedBaseCharacter->GetIKPelvisOffset()), 0.f, 0.f);
	PelvisEffectorLocation = FVector(0.f, 0.f, CachedBaseCharacter->GetIKPelvisOffset());

	AimRotation = CachedBaseCharacter->GetAimOffset();
	
	const UCharacterEquipmentComponent* CharacterEquipment = CachedBaseCharacter->GetCharacterEquipmentComponent();
	CurrentEquippedItemType = CharacterEquipment->GetCurrentEquippedItemType();

	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipment->GetCurrentRangeWeapon();
	if(IsValid(CurrentRangeWeapon))
	{
		ForeGripSocketTransform = CurrentRangeWeapon->GetForeGripTransform();
	}
}
