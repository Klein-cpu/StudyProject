// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"

#include "AIController.h"
#include "GameCodeTypes.h"
#include "Actors/Environment/PlatformTrigger.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Actors/Interactive/InteractiveActor.h"
#include "Actors/Interactive/Environment/Ladder.h"
#include "Actors/Interactive/Environment/Zipline.h"
#include "Actors/Interactive/Interface/IInteractable.h"
#include "Components/CapsuleComponent.h"
#include "Components/LedgeDetectorComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "Components/MovementComponents/BaseCharacterMovementComponent.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "UI/Widget/World/AttributeProgressBar.h"


//Basic Methods
ABaseCharacter::ABaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	BaseCharacterMovement = StaticCast<UBaseCharacterMovementComponent*>(GetCharacterMovement());
	BaseMesh = StaticCast<USkeletalMeshComponent*>(GetMesh());

	LedgeDetectorComponent = CreateDefaultSubobject<ULedgeDetectorComponent>(TEXT("LedgeDetector"));
	
	AvailableInteractiveActors.Reserve(10);

	GetMesh()->CastShadow = true;
	GetMesh()->bCastDynamicShadow = true;

	CharacterAttributesComponent = CreateDefaultSubobject<UCharacterAttributesComponent>(TEXT("CharacterAttributes"));
	CharacterEquipmentComponent = CreateDefaultSubobject<UCharacterEquipmentComponent>(TEXT("CharacterEquipment"));
	CharacterInventoryComponent = CreateDefaultSubobject<UCharacterInventoryComponent>(TEXT("CharacterInventory"));

	HealthBarProgressComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarProgressComponent"));
	HealthBarProgressComponent->SetupAttachment(GetCapsuleComponent());
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TryChangeSprintState();
	
	UpdateIKSettings(DeltaTime);
	
	if(GetCharacterMovement()->MovementMode == MOVE_Walking)
	{
		BaseCharacterMovement->bCanWallRun = true;
	}

	TraceLineOfSight();
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, bIsMantling);
	DOREPLIFETIME(ABaseCharacter, bIsSliding);
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AAIController* AIController = Cast<AAIController>(NewController);

	if(IsValid(AIController))
	{
		FGenericTeamId TeamId(static_cast<uint8>(Team));
		AIController->SetGenericTeamId(TeamId);
	}
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ABaseCharacter::OnPlayerCapsuleHit);
	GetCharacterMovement()->SetPlaneConstraintEnabled(true);

	CharacterAttributesComponent->OnDeathEvent.AddUObject(this, &ABaseCharacter::OnDeath);
	CharacterAttributesComponent->OutOfStaminaEvent.AddUObject(this, &ABaseCharacter::SetIsOutOfStamina);

	InitializeHealthProgress();
}

void ABaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if(OnInteractableObjectFound.IsBound())
	{
		OnInteractableObjectFound.Unbind();
	}
	
	Super::EndPlay(EndPlayReason);
}


//Basic Movement
void ABaseCharacter::StandUp()
{
	if(bIsSliding)
	{
		return;
	}
	
	if(BaseCharacterMovement->IsProning())
	{
		UnProne(false);
	}
	if(BaseCharacterMovement->IsCrouching())
	{
		UnCrouch();
		BaseCharacterMovement->bWantsCrouchFromProne = false;
	}
}


//Ladder
void ABaseCharacter::ClimbLadderUp(float Value)
{
	if(GetBaseCharacterMovementComponent()->IsOnLadder() && !FMath::IsNearlyZero(Value))
	{
		FVector LadderUpVector = GetBaseCharacterMovementComponent()->GetCurrentLadder()->GetActorUpVector();
		AddMovementInput(LadderUpVector, Value);
	}
}

void ABaseCharacter::InteractWithLadder()
{
	if(GetBaseCharacterMovementComponent()->IsOnLadder())
	{
		GetBaseCharacterMovementComponent()->DetachFromLadder(EDetachFromLadderMethod::JumpOff);
	}
	else
	{
		const ALadder* AvailableLadder = GetAvailableLadder();
		if(IsValid(AvailableLadder))
		{
			if(AvailableLadder->GetIsOnTop())
			{
				PlayAnimMontage(AvailableLadder->GetAttachFromTopAnimMontage());
			}
			GetBaseCharacterMovementComponent()->AttachToLadder(AvailableLadder);
		}
	}
}

const ALadder* ABaseCharacter::GetAvailableLadder() const
{
	const ALadder* Result = nullptr;
	for(auto InteractiveActor : AvailableInteractiveActors)
	{
		if(InteractiveActor->IsA<ALadder>())
		{
			Result = StaticCast<const ALadder*>(InteractiveActor);
			break;
		}
	}
	return Result;
}


//Zipline
void ABaseCharacter::AttachToZipline()
{
	const AZipline* AvailableZipline = GetAvailableZipline();
	if(IsValid(AvailableZipline))
	{
		GetBaseCharacterMovementComponent()->AttachToZipline(AvailableZipline);
	}
}

const AZipline* ABaseCharacter::GetAvailableZipline() const
{
	const AZipline* Result = nullptr;
	for(auto InteractiveActor : AvailableInteractiveActors)
	{
		if(InteractiveActor->IsA<AZipline>())
		{
			Result = StaticCast<const AZipline*>(InteractiveActor);
			break;
		}
	}
	return Result;
}

//Crouch
void ABaseCharacter::ChangeCrouchState()
{
	if(!BaseCharacterMovement->IsCrouching() && !BaseCharacterMovement->IsProning() && !bIsSliding)
	{
		Crouch();
	}
}

//Prone
void ABaseCharacter::RecalculateBaseEyeHeight()
{
	if(!bIsProned)
	{
		Super::RecalculateBaseEyeHeight();
	}
	else
	{
		BaseEyeHeight = PronedEyeHeight;
	}
	
}

void ABaseCharacter::ChangeProneState()
{
	if(!BaseCharacterMovement->IsProning() && BaseCharacterMovement->IsCrouching())
	{
		Prone();
	}
	else if(BaseCharacterMovement->IsProning() && !BaseCharacterMovement->IsCrouching())
	{
		UnProne(true); 
	}
}

bool ABaseCharacter::CanProne() const
{
	return !bIsProned && GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics() && !bIsSliding;
}

void ABaseCharacter::Prone()
{
	if(BaseCharacterMovement)
	{
		if(CanProne())
		{
			BaseCharacterMovement->bWantsToProne = true;
			BaseCharacterMovement->bWantsCrouchFromProne = false;
		}
	}
	
}

void ABaseCharacter::UnProne(bool bCrouchWanted)
{
	if(BaseCharacterMovement)
	{
		BaseCharacterMovement->bWantsToProne = false;
		if(bCrouchWanted)
		{
			BaseCharacterMovement->bWantsCrouchFromProne = true;
		}
	}
	
}

void ABaseCharacter::OnStartProne(float HeightAdjust, float ScaledHeightAdjust)
{
	RecalculateBaseEyeHeight();

	const ABaseCharacter* DefaultChar = GetDefault<ABaseCharacter>(GetClass());
	if(BaseMesh && DefaultChar->BaseMesh)
	{
		FVector& MeshRelativeLocation = BaseMesh->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z += HeightAdjust;
		BaseTranslationOffset.Z += MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->BaseTranslationOffset.Z + HeightAdjust;
	}
	
}

void ABaseCharacter::OnEndProne(float HeightAdjust, float ScaledHeightAdjust)
{
	RecalculateBaseEyeHeight();
	
	const ABaseCharacter* DefaultChar = GetDefault<ABaseCharacter>(GetClass());
	if(BaseMesh && DefaultChar->BaseMesh)
	{
		FVector& MeshRelativeLocation = BaseMesh->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z -= HeightAdjust;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->BaseTranslationOffset.Z;
	}
}

//Jump
bool ABaseCharacter::CanJumpInternal_Implementation() const
{
	return (Super::CanJumpInternal_Implementation() || bIsCrouched) && !BaseCharacterMovement->IsOutOfStamina() && !BaseCharacterMovement->IsProning() && !BaseCharacterMovement->IsMantling() && !bIsSliding;
}

void ABaseCharacter::Jump()
{
	if(BaseCharacterMovement->IsWallRunning())
	{
		BaseCharacterMovement->JumpFromWall();
	}
	else
	{
		Super::Jump();
	}
}

//Falling
void ABaseCharacter::Falling()
{
	Super::Falling();
	
	GetBaseCharacterMovementComponent()->bNotifyApex = true;
}

void ABaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	const float FallingHeight = (CurrentFallApex - GetActorLocation()).Z; 
	
	if(IsValid(FallDamageCurve))
	{
		float DamageAmount = FallDamageCurve->GetFloatValue(FallingHeight * .01f);
		TakeDamage(DamageAmount, FDamageEvent(), GetController(), Hit.GetActor());
	}
	
	if(IsValid(HardLandAnimMontage) && FallingHeight > MinHardLandHeight && CharacterAttributesComponent->IsAlive())
	{
		float Duration = PlayAnimMontage(HardLandAnimMontage);
		GetBaseCharacterMovementComponent()->DisableMovement();
		GetWorld()->GetTimerManager().SetTimer(HardLandMontageTimer, this, &ABaseCharacter::EnableMovement, Duration, false);
	}
}

void ABaseCharacter::SetIsOutOfStamina(bool bIsOutOfStamina_In)
{
	BaseCharacterMovement->SetIsOutOfStamina(bIsOutOfStamina_In);
}

const UCharacterEquipmentComponent* ABaseCharacter::GetCharacterEquipmentComponent() const
{
	return CharacterEquipmentComponent;
}

UCharacterEquipmentComponent* ABaseCharacter::GetCharacterEquipmentComponent_Mutable()
{
	return CharacterEquipmentComponent;
}

UCharacterAttributesComponent* ABaseCharacter::GetCharacterAttributesComponent() const
{
	return CharacterAttributesComponent;
}

void ABaseCharacter::StartFire()
{
	if(CharacterEquipmentComponent->IsEquipping())
	{
		return;
	}
	
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if(IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StartFire();
	}
}

void ABaseCharacter::StopFire()
{
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if(IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopFire();
	}
}

void ABaseCharacter::StartAiming()
{
	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if(!IsValid(CurrentRangeWeapon))
	{
		return;
	}
	
	bIsAiming = true;
	CurrentAimingMovementSpeed = CurrentRangeWeapon->GetAimMovementMaxSpeed();
	CurrentRangeWeapon->StartAim();
	OnStartAiming();
}

void ABaseCharacter::StopAiming()
{
	if(!bIsAiming)
	{
		return;
	}
	
	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();

	bIsAiming = false;
	CurrentAimingMovementSpeed = 0.f;
	
	CurrentRangeWeapon->StopAim();

	OnStopAiming();
}

bool ABaseCharacter::IsAiming() const
{
	return bIsAiming;
}

float ABaseCharacter::GetAimingMovementSpeed() const
{
	return CurrentAimingMovementSpeed;
}

void ABaseCharacter::OnStartAiming_Implementation()
{
	OnStartAimingInternal();
}

void ABaseCharacter::OnStopAiming_Implementation()
{
	OnStopAimingInternal();
}

void ABaseCharacter::Reload() const
{
	if(IsValid(CharacterEquipmentComponent->GetCurrentRangeWeapon()))
	{
		CharacterEquipmentComponent->ReloadCurrentWeapon();
	}
}

void ABaseCharacter::ChangeWeaponRegime()
{
	CharacterEquipmentComponent->ChangeCurrentWeaponRegime();
}

FRotator ABaseCharacter::GetAimOffset() const
{
	const FVector AimDirectionWorld = GetBaseAimRotation().Vector();
	const FVector AimDirectionLocal = GetTransform().InverseTransformVectorNoScale(AimDirectionWorld);
	const FRotator Result = AimDirectionLocal.ToOrientationRotator();
	return Result;
}

void ABaseCharacter::NextItem()
{
	CharacterEquipmentComponent->EquipNextItem();
}

void ABaseCharacter::PreviousItem()
{
	CharacterEquipmentComponent->EquipPreviousItem();
}

void ABaseCharacter::EquipPrimaryItem()
{
	CharacterEquipmentComponent->EquipItemInSlot(EEquipmentSlots::PrimaryItemSlot);
}

void ABaseCharacter::PrimaryMeleeAttack()
{
	AMeleeWeaponItem* CurrentMeleeWeapon = CharacterEquipmentComponent->GetCurrentMeleeWeapon();
	if(IsValid(CurrentMeleeWeapon))
	{
		CurrentMeleeWeapon->StartAttack(EMeleeAttackTypes::PrimaryAttack);
	}
}

void ABaseCharacter::SecondaryMeleeAttack()
{
	AMeleeWeaponItem* CurrentMeleeWeapon = CharacterEquipmentComponent->GetCurrentMeleeWeapon();
	if(IsValid(CurrentMeleeWeapon))
	{
		CurrentMeleeWeapon->StartAttack(EMeleeAttackTypes::SecondaryAttack);
	}
}

void ABaseCharacter::Interact()
{
	if(LineOfSightObject.GetInterface())
	{
		LineOfSightObject->Interact(this);
	}
}

void ABaseCharacter::InitializeHealthProgress()
{
	UAttributeProgressBar* Widget = Cast<UAttributeProgressBar>(HealthBarProgressComponent->GetUserWidgetObject());
	if(!IsValid(Widget))
	{
		HealthBarProgressComponent->SetVisibility(false);
		return;
	}

	if(IsPlayerControlled() && IsLocallyControlled())
	{
		HealthBarProgressComponent->SetVisibility(false);
		return;
	}
	
	HealthBarProgressComponent->SetVisibility(true);

	CharacterAttributesComponent->OnHealthChangedEvent.AddUObject(Widget, &UAttributeProgressBar::SetProgressPercentage);
	CharacterAttributesComponent->OnDeathEvent.AddLambda([=](){ HealthBarProgressComponent->SetVisibility(false); });
	Widget->SetProgressPercentage(CharacterAttributesComponent->GetHealthPercent());
}

bool ABaseCharacter::PickUpItem(TWeakObjectPtr<UInventoryItem> ItemToPickUp)
{
	bool Result = false;

	if(CharacterInventoryComponent->HasFreeSlot())
	{
		CharacterInventoryComponent->AddItem(ItemToPickUp, 1);
		Result = true;
	}
	return Result;
}

void ABaseCharacter::UseInventory(APlayerController* PlayerController)
{
	if(!IsValid(PlayerController))
	{
		return;
	}

	if(!CharacterInventoryComponent->IsViewVisible())
	{
		CharacterInventoryComponent->OpenViewInventory(PlayerController);
		CharacterEquipmentComponent->OpenViewEquipment(PlayerController);
		PlayerController->SetInputMode(FInputModeGameAndUI{});
		PlayerController->bShowMouseCursor = true;
	}
	else
	{
		CharacterInventoryComponent->CloseViewInventory();
		CharacterEquipmentComponent->CloseViewEquipment();
		PlayerController->SetInputMode(FInputModeGameOnly{});
		PlayerController->bShowMouseCursor = false;
	}
}

FGenericTeamId ABaseCharacter::GetGenericTeamId() const
{
	return FGenericTeamId(static_cast<uint8>(Team));
}

void ABaseCharacter::EnableMovement()
{
	GetBaseCharacterMovementComponent()->SetMovementMode(MOVE_Walking);
}

void ABaseCharacter::NotifyJumpApex()
{
	Super::NotifyJumpApex();

	CurrentFallApex = GetActorLocation();
}

//Swimming
bool ABaseCharacter::IsSwimmingUnderWater() const
{
	bool Result = false;
	
	if(GetCharacterMovement()->IsSwimming())
	{
		APhysicsVolume* Volume = GetCharacterMovement()->GetPhysicsVolume();
		float VolumeTopPlane = Volume->GetComponentsBoundingBox().Max.Z;
		FVector HeadPosition = GetMesh()->GetSocketLocation(SocketHead);
		if(VolumeTopPlane > HeadPosition.Z)
		{
			Result = true;
		}
	}
	return Result;
}

//Mantling
void ABaseCharacter::Mantle(bool bForce /*= false*/)
{
	if(!CanMantle() && !bForce)
	{
		return;
	}
	
	FLedgeDescription LedgeDescription;

	if(LedgeDetectorComponent->DetectLedge(LedgeDescription))
	{
		const ABaseCharacter* DefaultCharacter = GetDefault<ABaseCharacter>(GetClass());

		bIsMantling = true;
		
		FMantlingMovementParameters MantlingParameters;
		MantlingParameters.InitialLocation = FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		MantlingParameters.InitialRotation = GetActorRotation();
		MantlingParameters.TargetLocation = LedgeDescription.Location;
		MantlingParameters.TargetRotation = LedgeDescription.Rotation;
		MantlingParameters.InitialTargetMeshLocation = LedgeDescription.HittedMesh->GetComponentLocation();
		MantlingParameters.TargetMesh = LedgeDescription.HittedMesh;

		float MantlingHeight = (MantlingParameters.TargetLocation - MantlingParameters.InitialLocation).Z;

		const FMantlingSettings& MantlingSettings = GetMantlingSettings(MantlingHeight);
	
		float MinRange;
		float MaxRange;
		MantlingSettings.MantlingCurve->GetTimeRange(MinRange, MaxRange);

		MantlingParameters.MantlingCurve = MantlingSettings.MantlingCurve;
	
		MantlingParameters.Duration = MaxRange - MinRange;

		FVector2D SourceRange(MantlingSettings.MinHeight, MantlingSettings.MaxHeight);
		FVector2D TargetRange(MantlingSettings.MinHeightStartTime, MantlingSettings.MaxHeightStartTime);
		MantlingParameters.StartTime = FMath::GetMappedRangeValueClamped(SourceRange, TargetRange, MantlingHeight);

		MantlingParameters.InitialAnimationLocation = MantlingParameters.TargetLocation - MantlingSettings.AnimationCorrectionZ * FVector::UpVector + MantlingSettings.AnimationCorrectionXY * LedgeDescription.LedgeNormal;

		if(IsLocallyControlled() || GetLocalRole() == ROLE_Authority)
		{
			GetBaseCharacterMovementComponent()->StartMantle(MantlingParameters);
		}
	
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(MantlingSettings.MantlingMontage, 1.f, EMontagePlayReturnType::Duration, MantlingParameters.StartTime);
		OnMantle(MantlingSettings, MantlingParameters.StartTime);
	}
}

bool ABaseCharacter::CanMantle() const
{
	return ABaseCharacter::CanJumpInternal_Implementation() || BaseCharacterMovement->IsSwimming();
}

void ABaseCharacter::OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime)
{
	
}

//Interactive Actors
void ABaseCharacter::RegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActors.AddUnique(InteractiveActor);
}

void ABaseCharacter::UnregisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActors.RemoveSingleSwap(InteractiveActor);
}

//Slide
void ABaseCharacter::Slide(bool bForce)
{
	if(!CanSlide() && !bForce)
	{
		return;
	}
	
	bIsSliding = true;
	if(IsLocallyControlled())
	{
		GetBaseCharacterMovementComponent()->StartSlide();
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(SlideMontage, 1.f, EMontagePlayReturnType::Duration);
}

void ABaseCharacter::StopSlide()
{
	bIsSliding = false;
	GetBaseCharacterMovementComponent()->StopSlide();
}

bool ABaseCharacter::CanSlide()
{
	return BaseCharacterMovement->IsSprinting() && !bIsSliding;
}

void ABaseCharacter::OnStartSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	const ABaseCharacter* DefaultChar = GetDefault<ABaseCharacter>(GetClass());
	if(BaseMesh && DefaultChar->BaseMesh)
	{
		FVector& MeshRelativeLocation = BaseMesh->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z += HalfHeightAdjust;
		BaseTranslationOffset.Z += MeshRelativeLocation.Z;
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Black, FString::Printf(TEXT("HalfHeightAdjust = %.2f"), HalfHeightAdjust));
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->BaseTranslationOffset.Z + HalfHeightAdjust;
	}
}

void ABaseCharacter::OnEndSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	const ABaseCharacter* DefaultChar = GetDefault<ABaseCharacter>(GetClass());
	if(BaseMesh && DefaultChar->BaseMesh)
	{
		FVector& MeshRelativeLocation = BaseMesh->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z -= HalfHeightAdjust;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->BaseTranslationOffset.Z;
	}
}

void ABaseCharacter::OnRep_IsSliding(bool bWasSliding)
{
	if(GetLocalRole() == ROLE_SimulatedProxy && !bWasSliding && bIsSliding)
	{
		Slide(true);
	}
}

void ABaseCharacter::OnRep_IsMantling(bool bWasMantling)
{
	if(GetLocalRole() == ROLE_SimulatedProxy && !bWasMantling && bIsMantling)
	{
		Mantle(true);
	}
}

//Sprint
void ABaseCharacter::StartSprint()
{
	bIsSprintRequested = true;
	if(bIsCrouched)
	{
		UnCrouch();
	}
}

void ABaseCharacter::StopSprint()
{
	bIsSprintRequested = false;
}

void ABaseCharacter::OnDeath()
{
	float Duration = PlayAnimMontage(OnDeathAnimMontage);
	if(Duration == 0.f)
	{
		EnableRagdoll();
	}
	GetCharacterMovement()->DisableMovement();
}

void ABaseCharacter::EnableRagdoll()
{
	GetMesh()->SetCollisionProfileName(CollisionProfileRagdoll);
	GetMesh()->SetSimulatePhysics(true);
}


void ABaseCharacter::OnStartAimingInternal()
{
	if(OnAimingStateChangedEvent.IsBound())
	{
		OnAimingStateChangedEvent.Broadcast(true);
	}
}

void ABaseCharacter::OnStopAimingInternal()
{
	if(OnAimingStateChangedEvent.IsBound())
	{
		OnAimingStateChangedEvent.Broadcast(false);
	}
}

void ABaseCharacter::TraceLineOfSight()
{
	if(!IsPlayerControlled())
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	APlayerController* PlayerController = GetController<APlayerController>();
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector ViewDirection = ViewRotation.Vector();
	FVector TraceEnd = ViewLocation + ViewDirection * LineOfSightDistance;

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd, ECC_Visibility);
	if(LineOfSightObject.GetObject() != HitResult.GetActor())
	{
		LineOfSightObject = HitResult.GetActor();

		FName ActionName;
		if(LineOfSightObject.GetInterface())
		{
			ActionName = LineOfSightObject->GetActionEventName();
		}
		else
		{
			ActionName = FName(NAME_None);
		}
		OnInteractableObjectFound.ExecuteIfBound(ActionName);
	}
	
}

void ABaseCharacter::TryChangeSprintState()
{
	if(bIsSprintRequested && !GetBaseCharacterMovementComponent()->IsSprinting() && CanSprint())
	{
		BaseCharacterMovement->StartSprint();
		OnSprintStart();
	}
	if(!bIsSprintRequested && GetBaseCharacterMovementComponent()->IsSprinting())
	{
		BaseCharacterMovement->StopSprint();
		OnSprintEnd();
	}
}

bool ABaseCharacter::CanSprint()
{
	return !BaseCharacterMovement->IsOutOfStamina() && !BaseCharacterMovement->IsProning() && BaseCharacterMovement->GetLastUpdateVelocity().Size() > 0.f  && !bIsSliding && !BaseCharacterMovement->IsWallRunning();
}

//Inverse Kinematics
void ABaseCharacter::UpdateIKSettings(float DeltaTime)
{
	IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, GetIKOffsetForASocket(RightFootSocketName), DeltaTime, IKInterpSpeed);
	IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, GetIKOffsetForASocket(LeftFootSocketName), DeltaTime, IKInterpSpeed);
	IKPelvisOffset = FMath::FInterpTo(IKPelvisOffset, CalculateIKPelvisOffset(), DeltaTime, IKInterpSpeed);
}

float ABaseCharacter::GetIKOffsetForASocket(const FName& SocketName)
{
	float Result = 0.f;

	float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	
	FVector SocketLocation = GetMesh()->GetSocketLocation(SocketName);
	FVector TraceStart(SocketLocation.X, SocketLocation.Y, GetActorLocation().Z);
	FVector TraceEnd = TraceStart - (CapsuleHalfHeight + IKTraceExtendDistance) * FVector::UpVector;

	FHitResult HitResult;
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	
	const FVector FootSizeBox = FVector(1.f, 15.f, 7.f);
	if(UKismetSystemLibrary::BoxTraceSingle(GetWorld(), TraceStart, TraceEnd, FootSizeBox, GetMesh()->GetSocketRotation(SocketName), TraceType, true, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, HitResult, true))
	{
		Result = TraceStart.Z- CapsuleHalfHeight - HitResult.Location.Z;
	}
	
	return Result;
}

float ABaseCharacter::CalculateIKPelvisOffset()
{
	return IKRightFootOffset > IKLeftFootOffset ? -IKRightFootOffset : -IKLeftFootOffset;
}

//Mantling
const FMantlingSettings& ABaseCharacter::GetMantlingSettings(float LedgeHeight) const
{
	
	return LedgeHeight > LowMantleMaxHeight ? HighMantleSettings : LowMantleSettings;
}

//WallRun
void ABaseCharacter::OnPlayerCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	FVector HitNormal = Hit.ImpactNormal;

	if(!IsSurfaceWallRunable(HitNormal))
	{
		return;
	}

	if(!GetCharacterMovement()->IsFalling())
	{
		return;
	}
	
	EWallRunSide Side = EWallRunSide::None;
	FVector Direction = FVector::ZeroVector;
	BaseCharacterMovement->GetWallRunSideAndDirection(HitNormal, Side, Direction);

	if(!AreRequiredKeysDown())
	{
		return;
	}
	
	if(!BaseCharacterMovement->bCanWallRun)
	{
		return;
	}

	BaseCharacterMovement->StartWallRun(Side, Direction);
}

bool ABaseCharacter::IsSurfaceWallRunable(const FVector& SurfaceNormal) const
{
	if(SurfaceNormal.Z > GetCharacterMovement()->GetWalkableFloorZ() || SurfaceNormal.Z < -0.005f)
	{
		return false;
	}
	return true;
}

bool ABaseCharacter::AreRequiredKeysDown() const
{
	if(ForwardAxis < 0.1f)
	{
		return false;
	}

	if(RightAxis < -0.1f)
	{
		return false;
	}

	if(RightAxis > 0.1f)
	{
		return false;
	}

	return true;
}
