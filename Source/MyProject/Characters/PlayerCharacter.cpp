// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Camera/CameraComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

//Basic Methods
APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring arm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = 1;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);

	Team = ETeams::Player;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultArmLength = SpringArmComponent->TargetArmLength;
	
	if(IsValid(SprintTimelineCurve))
	{
		FOnTimelineFloatStatic SprintTimelineUpdate;
		SprintTimelineUpdate.BindUObject(this, &APlayerCharacter::SprintTimelineUpdate);
		SprintTimeline.AddInterpFloat(SprintTimelineCurve, SprintTimelineUpdate);
	}

	if(IsValid(AimTimelineCurve))
	{
		FOnTimelineFloatStatic AimTimelineUpdate;
		AimTimelineUpdate.BindUObject(this, &APlayerCharacter::AimTimelineUpdate);
		AimTimeline.AddInterpFloat(AimTimelineCurve, AimTimelineUpdate);
	}
	
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SprintTimeline.TickTimeline(DeltaSeconds);
	AimTimeline.TickTimeline(DeltaSeconds);
}

//BasicMovement
void APlayerCharacter::MoveForward(float Value)
{
	Super::MoveForward(Value);
	if((GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling()) && !FMath::IsNearlyZero(Value, 1e-6f))
	{
		FRotator YawRotator(0.f, GetControlRotation().Yaw, 0.f);
		FVector ForwardVector = YawRotator.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	Super::MoveRight(Value);
	if((GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling()) && !FMath::IsNearlyZero(Value, 1e-6f))
	{
		FRotator YawRotator(0.f, GetControlRotation().Yaw, 0.f);
		FVector RightVector = YawRotator.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
}

void APlayerCharacter::Turn(float Value)
{
	Super::Turn(Value);

	ARangeWeaponItem* CurrentEquippedWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();

	if(IsValid(CurrentEquippedWeapon) && IsAiming())
	{
		Value *= AimTurnModifier;
	}
	
	AddControllerYawInput(Value);
}

void APlayerCharacter::LookUp(float Value)
{
	Super::LookUp(Value);
	
	ARangeWeaponItem* CurrentEquippedWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();

	if(IsValid(CurrentEquippedWeapon) && IsAiming())
	{
		Value *= AimLookUpModifier;
	}
	
	AddControllerPitchInput(Value);
}

//Crouch
void APlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SpringArmComponent->TargetOffset += FVector(0.f, 0.f, HalfHeightAdjust);
}

void APlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SpringArmComponent->TargetOffset -= FVector(0.f, 0.f, HalfHeightAdjust);
}

//Prone
void APlayerCharacter::OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
	
	SpringArmComponent->TargetOffset += FVector(0.f, 0.f, HalfHeightAdjust);
}

void APlayerCharacter::OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
	
	SpringArmComponent->TargetOffset -= FVector(0.f, 0.f, HalfHeightAdjust);
}

//Slide
void APlayerCharacter::OnStartSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);
	
	SpringArmComponent->TargetOffset += FVector(0.f, 0.f, HalfHeightAdjust);
}

void APlayerCharacter::OnEndSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);
	
	SpringArmComponent->TargetOffset -= FVector(0.f, 0.f, HalfHeightAdjust);
}

//Swim
void APlayerCharacter::SwimForward(float Value)
{
	Super::SwimForward(Value);
	if(GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value, 1e-6f))
	{
		FRotator PitchYawRotator(GetControlRotation().Pitch, GetControlRotation().Yaw, 0.f);
		FVector ForwardVector = PitchYawRotator.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, Value);
	}
	
}

void APlayerCharacter::SwimRight(float Value)
{
	Super::SwimRight(Value);
	if(GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value, 1e-6f))
	{
		FRotator YawRotator(0.f, GetControlRotation().Yaw, 0.f);
		FVector RightVector = YawRotator.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
	
}

void APlayerCharacter::SwimUp(float Value)
{
	Super::SwimUp(Value);
	if(GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value, 1e-6f))
	{
		AddMovementInput(FVector::UpVector, Value);
	}
}

//Jump
void APlayerCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	if(bIsCrouched)
	{
		UnCrouch();
	}
}

//Sprint
void APlayerCharacter::OnSprintStart_Implementation()
{
	Super::OnSprintStart_Implementation();
	SprintTimeline.Play();
}

void APlayerCharacter::OnSprintEnd_Implementation()
{
	Super::OnSprintEnd_Implementation();
	SprintTimeline.Reverse();
}

void APlayerCharacter::SprintTimelineUpdate(float Alpha) const
{
	const float SpringArmLength = FMath::Lerp(DefaultArmLength, SprintArmLength, Alpha);
	SpringArmComponent->TargetArmLength = SpringArmLength;
}

//Aim
void APlayerCharacter::OnStartAimingInternal()
{
	Super::OnStartAimingInternal();
	AimTimeline.Play();
}

void APlayerCharacter::OnStopAimingInternal()
{
	Super::OnStopAimingInternal();
	AimTimeline.Reverse();
}

void APlayerCharacter::AimTimelineUpdate(float Alpha) const
{
	APlayerController* PlayerController = GetController<APlayerController>();
	if(!IsValid(PlayerController))
	{
		return;
	}
	
	APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
	if(!IsValid(CameraManager))
	{
		return;
	}
	
	const float CameraFOV = FMath::Lerp(CameraManager->DefaultFOV, CharacterEquipmentComponent->GetCurrentRangeWeapon()->GetAimFOV(), Alpha);
	CameraManager->SetFOV(CameraFOV);

	const float TurnModifier = FMath::Lerp(1.f, CharacterEquipmentComponent->GetCurrentRangeWeapon()->GetAimTurnModifier(), Alpha);
	AimTurnModifier = TurnModifier;
	
	const float LookUpModifier = FMath::Lerp(1.f, CharacterEquipmentComponent->GetCurrentRangeWeapon()->GetAimLookUpModifier(), Alpha);
	AimLookUpModifier = LookUpModifier;
}