// Fill out your copyright notice in the Description page of Project Settings.


#include "FPPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Controllers/BaseCharacterPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "MyProject/GameCodeTypes.h"
#include "MyProject/Components/MovementComponents/BaseCharacterMovementComponent.h"

AFPPlayerCharacter::AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	FirstPersonMeshComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -86.f));
	FirstPersonMeshComponent->CastShadow = false;
	FirstPersonMeshComponent->bCastDynamicShadow = false;
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);
	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, SocketFPCamera);
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastHiddenShadow = true;

	CameraComponent->bAutoActivate = false;

	SpringArmComponent->bAutoActivate = false;
	SpringArmComponent->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;

	bUseControllerRotationYaw = true;
}

void AFPPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(IsFPMontagePlaying() && BasePlayerController.IsValid())
	{
		FRotator TargetControlRotation = BasePlayerController->GetControlRotation();
		TargetControlRotation.Pitch = 0.f;
		float BlendSpeed = 300.f;
		TargetControlRotation = FMath::RInterpTo(BasePlayerController->GetControlRotation(), TargetControlRotation, DeltaSeconds, BlendSpeed);
		BasePlayerController->SetControlRotation(TargetControlRotation);
	}
	
}

void AFPPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	BasePlayerController = Cast<ABaseCharacterPlayerController>(NewController);
}

void AFPPlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& FirstPersonMeshRelativeLocation = FirstPersonMeshComponent->GetRelativeLocation_DirectMutable();
	FirstPersonMeshRelativeLocation.Z = DefaultCharacter->FirstPersonMeshComponent->GetRelativeLocation().Z + HalfHeightAdjust;
}

void AFPPlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& FirstPersonMeshRelativeLocation = FirstPersonMeshComponent->GetRelativeLocation_DirectMutable();
	FirstPersonMeshRelativeLocation.Z = DefaultCharacter->FirstPersonMeshComponent->GetRelativeLocation().Z;
}

FRotator AFPPlayerCharacter::GetViewRotation() const
{
	FRotator Result = Super::GetViewRotation();

	if(IsFPMontagePlaying())
	{
		FRotator SocketRotation = FirstPersonMeshComponent->GetSocketRotation(SocketFPCamera);
		Result.Pitch += SocketRotation.Pitch;
		Result.Yaw = SocketRotation.Yaw;
		Result.Roll = SocketRotation.Roll;
	}
	
	return Result;
	
}

void AFPPlayerCharacter::OnLadderStarted()
{
	if(BasePlayerController.IsValid())
	{
		BasePlayerController->SetIgnoreCameraPitch(true);
		bUseControllerRotationYaw = false;
		APlayerCameraManager* CameraManager = BasePlayerController->PlayerCameraManager;
		CameraManager->ViewPitchMin = LadderCameraMinPitch;
		CameraManager->ViewPitchMax = LadderCameraMaxPitch;
		CameraManager->ViewYawMin = LadderCameraMinYaw;
		CameraManager->ViewYawMax = LadderCameraMaxYaw;
	}
}

void AFPPlayerCharacter::OnLadderStopped()
{
	if(BasePlayerController.IsValid())
	{
		BasePlayerController->SetIgnoreCameraPitch(false);
		bUseControllerRotationYaw = true;
		APlayerCameraManager* CameraManager = BasePlayerController->PlayerCameraManager;
		APlayerCameraManager* DefaultCameraManager = CameraManager->GetClass()->GetDefaultObject<APlayerCameraManager>();
		CameraManager->ViewPitchMin = DefaultCameraManager->ViewPitchMin;
		CameraManager->ViewPitchMax = DefaultCameraManager->ViewPitchMax;
		CameraManager->ViewYawMin = DefaultCameraManager->ViewYawMin;
		CameraManager->ViewYawMax = DefaultCameraManager->ViewYawMax;
	}
}

void AFPPlayerCharacter::OnZiplineStarted()
{
	if(BasePlayerController.IsValid())
	{
		BasePlayerController->SetIgnoreCameraPitch(true);
		bUseControllerRotationYaw = false;
		APlayerCameraManager* CameraManager = BasePlayerController->PlayerCameraManager;
		CameraManager->ViewPitchMin = ZiplineCameraMinPitch;
		CameraManager->ViewPitchMax = ZiplineCameraMaxPitch;
		CameraManager->ViewYawMin = ZiplineCameraMinYaw;
		CameraManager->ViewYawMax = ZiplineCameraMaxYaw;
	}
}

void AFPPlayerCharacter::OnZiplineStopped()
{
	BasePlayerController->SetIgnoreCameraPitch(false);
	bUseControllerRotationYaw = true;
	APlayerCameraManager* CameraManager = BasePlayerController->PlayerCameraManager;
	APlayerCameraManager* DefaultCameraManager = CameraManager->GetClass()->GetDefaultObject<APlayerCameraManager>();
	CameraManager->ViewPitchMin = DefaultCameraManager->ViewPitchMin;
	CameraManager->ViewPitchMax = DefaultCameraManager->ViewPitchMax;
	CameraManager->ViewYawMin = DefaultCameraManager->ViewYawMin;
	CameraManager->ViewYawMax = DefaultCameraManager->ViewYawMax;
}

void AFPPlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if(GetBaseCharacterMovementComponent()->IsOnLadder())
	{
		OnLadderStarted();
	}
	else if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
	{
		OnLadderStopped();
	}

	if(GetBaseCharacterMovementComponent()->IsOnZipline())
	{
		OnZiplineStarted();
	}
	else if(PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Zipline)
	{
		OnZiplineStopped();
	}
}

void AFPPlayerCharacter::Falling()
{
	Super::Falling();
}

void AFPPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	
	const float FallingHeight = CurrentFallApex.Z - GetActorLocation().Z; 
	UAnimInstance* FPAnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	if(FallingHeight > MinHardLandHeight)
	{
		if(IsValid(FPAnimInstance))
		{
			if(BasePlayerController.IsValid())
			{
				BasePlayerController->SetIgnoreLookInput(true);
				BasePlayerController->SetIgnoreMoveInput(true);
			}
			float FPMontageDuration = FPAnimInstance->Montage_Play(FPHardLandAnimMontage, 1.f, EMontagePlayReturnType::Duration);		
			GetWorld()->GetTimerManager().SetTimer(FPMontageTimer, this, &AFPPlayerCharacter::OnFPMontageTimerElapsed, FPMontageDuration, false);
		}
	}
	
}

void AFPPlayerCharacter::OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime)
{
	Super::OnMantle(MantlingSettings, MantlingAnimationStartTime);
	UAnimInstance * FPAnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	if(IsValid(FPAnimInstance) && MantlingSettings.FPMantlingMontage)
	{
		if(BasePlayerController.IsValid())
		{
			BasePlayerController->SetIgnoreLookInput(true);
			BasePlayerController->SetIgnoreMoveInput(true);
		}
		float FPMontageDuration = FPAnimInstance->Montage_Play(MantlingSettings.FPMantlingMontage, 1.f, EMontagePlayReturnType::Duration, MantlingAnimationStartTime);		
		GetWorld()->GetTimerManager().SetTimer(FPMontageTimer, this, &AFPPlayerCharacter::OnFPMontageTimerElapsed, FPMontageDuration, false);
	}
}

void AFPPlayerCharacter::OnFPMontageTimerElapsed()
{
	if(BasePlayerController.IsValid())
	{
		BasePlayerController->SetIgnoreLookInput(false);
		BasePlayerController->SetIgnoreMoveInput(false);
	}
}

bool AFPPlayerCharacter::IsFPMontagePlaying() const
{
	UAnimInstance * FPAnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	return IsValid(FPAnimInstance) && FPAnimInstance->IsAnyMontagePlaying();
}
