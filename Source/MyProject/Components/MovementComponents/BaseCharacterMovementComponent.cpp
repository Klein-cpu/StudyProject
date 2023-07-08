// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "Engine/RendererSettings.h"
#include "GameFramework/Character.h"
#include "MyProject/Actors/Interactive/Environment/Ladder.h"
#include "MyProject/Actors/Interactive/Environment/Zipline.h"
#include "MyProject/Characters/BaseCharacter.h"

//Network
FNetworkPredictionData_Client* UBaseCharacterMovementComponent::GetPredictionData_Client() const
{
	if(ClientPredictionData == nullptr)
	{
		UBaseCharacterMovementComponent* MutableThis = const_cast<UBaseCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Character_GC(*this);
	}
	return ClientPredictionData;
	
}

void UBaseCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bool bWasMantling = GetBaseCharacterOwner()->bIsMantling;
	bool bWasSliding = GetBaseCharacterOwner()->bIsSliding;
	bIsSprinting = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bool bIsMantling = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
	bool bIsSliding = (Flags & FSavedMove_Character::FLAG_Custom_2) != 0;
	
	if(GetBaseCharacterOwner()->GetLocalRole() == ROLE_Authority)
	{
		if(!bWasMantling && bIsMantling)
		{
			GetBaseCharacterOwner()->Mantle(true);
		}

		if(!bWasSliding && bIsSliding)
		{
			GetBaseCharacterOwner()->Slide();
		}
	}
}

//Related Methods
void UBaseCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	BaseCharacterOwner = Cast<ABaseCharacter>(CharacterOwner);
}

void UBaseCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
	if(IsProning() && !bWantsToProne)
	{
		UnProne(bWantsCrouchFromProne);
	}
	else if(!IsProning() && bWantsToProne)
	{
		Prone();
	}
}

void UBaseCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	if(bForceRotation)
	{
		FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
		CurrentRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): CurrentRotation"));

		FRotator DeltaRot = GetDeltaRotation(DeltaTime);
		DeltaRot.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): GetDeltaRotation"));

		// Accumulate a desired new rotation.
		const float AngleTolerance = 1e-3f;

		if (!CurrentRotation.Equals(ForcedTargetRotation, AngleTolerance))
		{
			FRotator DesiredRotation = ForcedTargetRotation;
			// PITCH
			if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
			{
				DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
			}

			// YAW
			if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
			{
				DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
			}

			// ROLL
			if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
			{
				DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
			}

			// Set the new rotation.
			DesiredRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
			MoveUpdatedComponent( FVector::ZeroVector, DesiredRotation, /*bSweep*/ false );
		}
		else
		{
			ForcedTargetRotation = FRotator::ZeroRotator;
			bForceRotation = false;
		}
		return;
	}
	if(IsOnLadder())
	{
		return;
	}
	Super::PhysicsRotation(DeltaTime);
	
}

ABaseCharacter* UBaseCharacterMovementComponent::GetBaseCharacterOwner() const
{
	return BaseCharacterOwner;
}

float UBaseCharacterMovementComponent::GetMaxSpeed() const
{
	float Result = Super::GetMaxSpeed();
	if(bIsSprinting)
	{
		Result = SprintSpeed;
	}
	else if(bIsOutOfStamina)
	{
		Result = OutOfStaminaSpeed;
	}
	else if(IsProning())
	{
		Result = MaxProneSpeed;
	}
	else if(IsOnLadder())
	{
		Result = ClimbingOnLadderMaxSpeed;
	}
	else if(IsOnZipline())
	{
		Result = ZiplineSlidingSpeed;
	}
	else if(IsWallRunning())
	{
		Result = MaxWallRunSpeed;
	}
	else if(IsSliding())
	{
		Result = SlideSpeed;
	}
	else if(IsValid(BaseCharacterOwner) ? BaseCharacterOwner->IsAiming() : false)
	{
		Result = BaseCharacterOwner->GetAimingMovementSpeed();
	}
	return Result;
}

//Getters
bool UBaseCharacterMovementComponent::IsProning() const
{
	return BaseCharacterOwner && BaseCharacterOwner->bIsProned;
}

bool UBaseCharacterMovementComponent::IsWallRunning() const
{
	return bIsWallRunning;
}

EWallRunSide UBaseCharacterMovementComponent::GetWallRunSide() const
{
	return CurrentWallRunSide;
}

bool UBaseCharacterMovementComponent::IsMantling() const 
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Mantling;
}

bool UBaseCharacterMovementComponent::IsOnZipline() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Zipline;
}

bool UBaseCharacterMovementComponent::IsOnLadder() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Ladder;
}

bool UBaseCharacterMovementComponent::IsSliding() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Slide;
}

//Mantling
void UBaseCharacterMovementComponent::StartMantle(const FMantlingMovementParameters& MantlingParameters)
{
	CurrentMantlingParameters = MantlingParameters;
	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Mantling);
}

void UBaseCharacterMovementComponent::EndMantle()
{
	GetBaseCharacterOwner()->bIsMantling = false;
	SetMovementMode(MOVE_Walking);
}

//Sprint
void UBaseCharacterMovementComponent::StartSprint()
{
	bIsSprinting = true;
	bForceMaxAccel = 1;
}

void UBaseCharacterMovementComponent::StopSprint()
{
	bIsSprinting = false;
	bForceMaxAccel = 0;
}

//Prone
void UBaseCharacterMovementComponent::Prone()
{
	if(!HasValidData())
	{
		return;
	}

	if(BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == PronedHalfHeight)
	{
		BaseCharacterOwner->OnStartProne(0.f, 0.f);
		return;
	}

	// Change collision size to proning dimensions
	const float ComponentScale = BaseCharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	// const float OldUnscaledRadius = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.
	const float ClampedPronedHalfHeight = FMath::Max3(0.f, PronedHalfHeight, PronedCapsuleRadius);
	BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(PronedCapsuleRadius, PronedHalfHeight);
	const float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedPronedHalfHeight);
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	BaseCharacterOwner->OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);

	
	
	BaseCharacterOwner->bIsCrouched = false;
	BaseCharacterOwner->bIsProned = true;
	bWantsToCrouch = false;
}

void UBaseCharacterMovementComponent::UnProne(bool bCrouchWanted)
{
	if (!HasValidData())
	{
		return;
	}

	float WantedHalfHeight;
	float WantedRadius;
	ABaseCharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ABaseCharacter>();
	
	if(bCrouchWanted)
	{
		WantedHalfHeight = CrouchedHalfHeight;
		WantedRadius = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	}
	else
	{
		WantedHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		WantedRadius = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	}
	
	if(BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
	{
		CharacterOwner->OnEndCrouch(0.f, 0.f);
		return;
	}

	const float CurrentPronedHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = WantedHalfHeight - OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	check(CharacterOwner->GetCapsuleComponent());

	const UWorld* MyWorld = GetWorld();
	const float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(ProneTrace), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);

	const FCollisionShape WantedCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	bool bEncroached = true;

	FVector WantedLocation = PawnLocation + FVector(0.f, 0.f, WantedCapsuleShape.GetCapsuleHalfHeight() - CurrentPronedHalfHeight);
	bEncroached = MyWorld->OverlapBlockingTestByChannel(WantedLocation, FQuat::Identity, CollisionChannel, WantedCapsuleShape, CapsuleParams, ResponseParam);

	if(bEncroached)
	{
		if(IsMovingOnGround())
		{
			const float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
			if(CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
			{
				WantedLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
				bEncroached = MyWorld->OverlapBlockingTestByChannel(WantedLocation, FQuat::Identity, CollisionChannel, WantedCapsuleShape, CapsuleParams, ResponseParam);
			}
		}
	}
	if(!bEncroached)
	{
		UpdatedComponent->MoveComponent(WantedLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	}

	if(bEncroached)
	{
		return;
	}
	
	BaseCharacterOwner->bIsProned = false;
	BaseCharacterOwner->bIsCrouched = bCrouchWanted;
	bWantsToCrouch = bCrouchWanted;
	
	//Changing capsule size
	BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(WantedRadius, WantedHalfHeight);

	AdjustProxyCapsuleSize();
	BaseCharacterOwner->OnEndProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
	
}

//Ziplines
void UBaseCharacterMovementComponent::AttachToZipline(const AZipline* Zipline)
{
	CurrentZipline = Zipline;

	FRotator TargetOrientationRotation = CurrentZipline->GetActorForwardVector().ToOrientationRotator();
	TargetOrientationRotation.Yaw = CurrentZipline->IsFirstPillarHigher() ? 0.f : 180.f;

	FVector WireForwardVector = CurrentZipline->GetWireForwardVector();
	
	float Projection = GetActorToCurrentZiplineProjection(GetActorLocation());
	FVector NewCharacterLocation = CurrentZipline->GetFirstPillarTop() + Projection * WireForwardVector -
		CharacterZiplineOffset * FVector::UpVector + 2.f * FVector::RightVector;
	
	GetOwner()->SetActorLocation(NewCharacterLocation);
	GetOwner()->SetActorRotation(TargetOrientationRotation);
	
	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Zipline);
}

void UBaseCharacterMovementComponent::DetachFromZipline()
{
	SetMovementMode(MOVE_Falling);
}

const AZipline* UBaseCharacterMovementComponent::GetCurrentZipline() const
{
	return CurrentZipline;
}

float UBaseCharacterMovementComponent::GetActorToCurrentZiplineProjection(const FVector& Location) const
{
	checkf(IsValid(CurrentZipline), TEXT("UBaseCharacterMovementComponent::GetCharacterToCurrentZiplineProjection() cannot be invoked when current zipline is null"));

	FVector ZiplineForwardVector = CurrentZipline->GetWireForwardVector();
	FVector ZiplineToCharacterDistance = GetActorLocation() - CurrentZipline->GetFirstPillarTop();
	return FVector::DotProduct(ZiplineForwardVector, ZiplineToCharacterDistance);
}

//Ladders
void UBaseCharacterMovementComponent::AttachToLadder(const ALadder* Ladder)
{
	CurrentLadder = Ladder;

	FRotator TargetOrientationRotation = CurrentLadder->GetActorForwardVector().ToOrientationRotator();
	TargetOrientationRotation.Yaw += 180.f;

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderForwardVector = CurrentLadder->GetActorForwardVector();
	
	float Projection = GetActorToCurrentLadderProjection(GetActorLocation());
	
	FVector NewCharacterLocation = CurrentLadder->GetActorLocation() + Projection * LadderUpVector + LadderToCharacterOffset * LadderForwardVector;

	if(CurrentLadder->GetIsOnTop())
	{
		NewCharacterLocation = CurrentLadder->GetAttachFromTopAnimMontageStartingLocation();
	}
	
	GetOwner()->SetActorLocation(NewCharacterLocation);
	GetOwner()->SetActorRotation(TargetOrientationRotation);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Ladder);
}

void UBaseCharacterMovementComponent::DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod)
{
	const FVector JumpDirection = CurrentLadder->GetActorForwardVector();
	const FVector JumpVelocity = JumpDirection * JumpOffFromLadderSpeed;
	switch (DetachFromLadderMethod)
	{
	case EDetachFromLadderMethod::JumpOff:
		
		SetMovementMode(MOVE_Falling);

		ForcedTargetRotation = JumpDirection.ToOrientationRotator();
		bForceRotation = true;
		
		Launch(JumpVelocity);
		break;
	case EDetachFromLadderMethod::ReachingTheTop:
		BaseCharacterOwner->Mantle(true);
		break;
	case EDetachFromLadderMethod::ReachingTheBottom:
		SetMovementMode(MOVE_Walking);
		break;
	case EDetachFromLadderMethod::Fall:
	default:
		break;
	}
}

const ALadder* UBaseCharacterMovementComponent::GetCurrentLadder()
{
	return CurrentLadder;
}

float UBaseCharacterMovementComponent::GetActorToCurrentLadderProjection(const FVector& Location) const
{
	checkf(IsValid(CurrentLadder), TEXT("UBaseCharacterMovementComponent::GetCharacterToCurrentLadderProjection() cannot be invoked when current ladder is null"));
	
	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderToCharacterDistance = GetActorLocation() - CurrentLadder->GetActorLocation();
	return FVector::DotProduct(LadderUpVector, LadderToCharacterDistance);
}

float UBaseCharacterMovementComponent::GetLadderSpeedRatio() const
{
	checkf(IsValid(CurrentLadder), TEXT("UBaseCharacterMovementComponent::GetLadderSpeedRatio() cannot be invoked when current ladder is null"));

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	return FVector::DotProduct(LadderUpVector, Velocity) / ClimbingOnLadderMaxSpeed;
}

//WallRun
void UBaseCharacterMovementComponent::StartWallRun(EWallRunSide Side, const FVector& Direction)
 {
	bIsWallRunning = true;
	bCanWallRun = false;
	
	CurrentWallRunSide = Side;
	CurrentWallRunDirection = Direction;

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_WallRun);
	
	SetPlaneConstraintNormal(FVector::UpVector);
	GetWorld()->GetTimerManager().SetTimer(WallRunTimer, this, &UBaseCharacterMovementComponent::StopWallRun, MaxWallRunTime);
}

void UBaseCharacterMovementComponent::JumpFromWall()
{
	FVector JumpDirection = FVector::ZeroVector;
	if(GetWallRunSide() == EWallRunSide::Right)
	{
		JumpDirection = FVector::CrossProduct(CurrentWallRunDirection, FVector::UpVector);
	}
	else
	{
		JumpDirection = FVector::CrossProduct(FVector::UpVector, CurrentWallRunDirection);
	}

	JumpDirection += FVector::UpVector;

	BaseCharacterOwner->LaunchCharacter(JumpZVelocity * JumpDirection.GetSafeNormal(), false, true);
		
	StopWallRun();
	bCanWallRun = true;
}

void UBaseCharacterMovementComponent::StopWallRun()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("Wallrun ended!"));
	bIsWallRunning = false;
	
	CurrentWallRunSide = EWallRunSide::None;
	CurrentWallRunDirection = FVector::ZeroVector;
	
	SetPlaneConstraintNormal(FVector::ZeroVector);
	SetMovementMode(MOVE_Falling);
}

void UBaseCharacterMovementComponent::GetWallRunSideAndDirection(const FVector& HitNormal, EWallRunSide& OutSide,
                                                                 FVector& OutDirection) const
{
	if(FVector::DotProduct(HitNormal, GetOwner()->GetActorRightVector()) > 0)
	{
		OutSide = EWallRunSide::Left;
		OutDirection = FVector::CrossProduct(HitNormal, FVector::UpVector).GetSafeNormal();
	}
	else
	{
		
		OutSide = EWallRunSide::Right;
		OutDirection = FVector::CrossProduct(FVector::UpVector, HitNormal).GetSafeNormal();
	}
}

//Slide
void UBaseCharacterMovementComponent::StartSlide()
{
	GetWorld()->GetTimerManager().SetTimer(SlidingTimer, this, &UBaseCharacterMovementComponent::StopSlide, SlideMaxTime);
	
	if(!HasValidData())
	{
		return;
	}

	if(BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == SlideCapsuleHalfHeight)
	{
		BaseCharacterOwner->OnStartSlide(0.f, 0.f);
		return;
	}

	// Change collision size to sliding dimensions
	const float ComponentScale = BaseCharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// const float OldUnscaledRadius = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.
	const float ClampedSlidedHalfHeight = FMath::Max3(0.f, SlideCapsuleHalfHeight, OldUnscaledRadius);
	BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, SlideCapsuleHalfHeight);
	const float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedSlidedHalfHeight);
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	BaseCharacterOwner->OnStartSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Slide);
	
}

void UBaseCharacterMovementComponent::StopSlide()
{
	BaseCharacterOwner->bIsCrouched = true;
	
	if (!HasValidData())
	{
		return;
	}

	ABaseCharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ABaseCharacter>();
	float WantedHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float WantedRadius = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	
	if(BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
	{
		CharacterOwner->OnEndCrouch(0.f, 0.f);
		return;
	}

	check(CharacterOwner->GetCapsuleComponent());
	const float CurrentPronedHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = WantedHalfHeight - OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();


	const UWorld* MyWorld = GetWorld();
	const float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(ProneTrace), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);

	const FCollisionShape WantedCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	bool bEncroached = true;

	FVector WantedLocation = PawnLocation + FVector(0.f, 0.f, WantedCapsuleShape.GetCapsuleHalfHeight() - CurrentPronedHalfHeight);
	bEncroached = MyWorld->OverlapBlockingTestByChannel(WantedLocation, FQuat::Identity, CollisionChannel, WantedCapsuleShape, CapsuleParams, ResponseParam);

	SetMovementMode(MOVE_Walking);
	if(bEncroached)
	{
		if(IsMovingOnGround())
		{
			const float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
			if(CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
			{
				WantedLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
				bEncroached = MyWorld->OverlapBlockingTestByChannel(WantedLocation, FQuat::Identity, CollisionChannel, WantedCapsuleShape, CapsuleParams, ResponseParam);
				WantedHalfHeight = CrouchedHalfHeight;
				WantedRadius = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
			}
		}
	}
	if(!bEncroached)
	{
		UpdatedComponent->MoveComponent(WantedLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	}

	if(bEncroached)
	{
		BaseCharacterOwner->bIsCrouched = bEncroached;
		bWantsToCrouch = bEncroached;
		return;
	}
	
	BaseCharacterOwner->bIsProned = false;
	
	//Changing capsule size
	BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(WantedRadius, WantedHalfHeight);

	AdjustProxyCapsuleSize();
	BaseCharacterOwner->OnEndSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

//Phys Custom Modes
void UBaseCharacterMovementComponent::PhysSliding(float DeltaTime, int32 Iterations)
{
	FVector PhysSlidingMovementVector = BaseCharacterOwner->GetActorForwardVector();
	BaseCharacterOwner->AddMovementInput(PhysSlidingMovementVector);

	CalcVelocity(DeltaTime,	GroundFriction, false, BrakingDecelerationWalking);
	FVector Delta = Velocity * DeltaTime;
	
	FHitResult OutHit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, OutHit);
}

void UBaseCharacterMovementComponent::PhysWallRun(float DeltaTime, int32 Iterations)
{
	CalcVelocity(DeltaTime, BrakingFriction, false, BrakingDecelerationWalking);
	FVector Delta = Velocity * DeltaTime;
	
	FHitResult HitOut;

	FVector LineTraceDirection = CurrentWallRunSide == EWallRunSide::Right ? GetOwner()->GetActorRightVector() : -GetOwner()->GetActorRightVector();
	float LineTraceLength = 200.f;
	
	FVector StartPosition = GetActorLocation();
	FVector EndPosition = StartPosition + LineTraceLength * LineTraceDirection;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	if(GetWorld()->LineTraceSingleByChannel(HitOut, StartPosition, EndPosition, ECC_Visibility, QueryParams))
	{
		FVector HitNormal = HitOut.ImpactNormal;
		EWallRunSide Side = EWallRunSide::None;
		FVector Direction = FVector::ZeroVector;
		GetWallRunSideAndDirection(HitNormal, Side, Direction);

		if(Side != CurrentWallRunSide)
		{
			StopWallRun();
		}
		else
		{
			CurrentWallRunDirection = Direction;
			Velocity = GetMaxSpeed() * CurrentWallRunDirection;
			GetOwner()->SetActorRotation(CurrentWallRunDirection.Rotation());
		}
	}
	else
	{
		StopWallRun();
	}
	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
}

void UBaseCharacterMovementComponent::PhysZipline(float DeltaTime, int32 Iterations)
{
	FVector PhysZiplineMovementVector = CurrentZipline->GetWireForwardVector() * (CurrentZipline->IsFirstPillarHigher() ? -1 : 1);
	BaseCharacterOwner->AddMovementInput(PhysZiplineMovementVector);
	
	CalcVelocity(DeltaTime,	GroundFriction, false, BrakingDecelerationWalking);
	FVector Delta = Velocity * DeltaTime;
	
	FVector NewLocation = GetActorLocation() + Delta;
	float NewLocationProjection = GetActorToCurrentZiplineProjection(NewLocation);
	if(CurrentZipline->IsFirstPillarHigher())
	{
		if(FMath::Abs(NewLocationProjection) > CurrentZipline->GetWireLength() - ZiplineEndOffset)
		{
			DetachFromZipline();
			return;
		}
	}
	else
	{
		if(FMath::Abs(NewLocationProjection) < ZiplineEndOffset)
		{
			DetachFromZipline();
			return;
		}
	}
	
	FHitResult OutHit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), false, OutHit);
	FVector NewCharacterLocation = CurrentZipline->GetFirstPillarTop() + NewLocationProjection * CurrentZipline->GetWireForwardVector() - CharacterZiplineOffset * FVector::UpVector + 5.f * GetActorLocation().RightVector;
	GetOwner()->SetActorLocation(FVector(NewCharacterLocation.X, NewCharacterLocation.Y, NewCharacterLocation.Z));
}

void UBaseCharacterMovementComponent::PhysLadder(float DeltaTime, int32 Iterations)
{
	CalcVelocity(DeltaTime, 1.f, false, ClimbingOnLadderBrakingDeceleration);
	FVector Delta = Velocity * DeltaTime;

	if(HasAnimRootMotion())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), false, Hit);
		return;
	}
	
	FVector NewLocation = GetActorLocation() + Delta;
	float NewLocationProjection = GetActorToCurrentLadderProjection(NewLocation);

	if(NewLocationProjection < MinLadderBottomOffset)
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheBottom);
		return;
	}
	else if
	(NewLocationProjection > (CurrentLadder->GetLadderHeight() - MaxLadderTopOffset))
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheTop);
		return;
	}
	
	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
}

void UBaseCharacterMovementComponent::PhysMantling(float DeltaTime, int32 Iterations)
{
	float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(MantlingTimer) + CurrentMantlingParameters.StartTime ;

	FVector MantlingCurveValue = CurrentMantlingParameters.MantlingCurve->GetVectorValue(ElapsedTime);

	float PositionAlpha = MantlingCurveValue.X;
	float XYCorrectionAlpha = MantlingCurveValue.Y;
	float ZCorrectionAlpha = MantlingCurveValue.Z;
				
	FVector CorrectedInitialLocation = FMath::Lerp(CurrentMantlingParameters.InitialLocation, CurrentMantlingParameters.InitialAnimationLocation, XYCorrectionAlpha);
	CorrectedInitialLocation.Z = FMath::Lerp(CurrentMantlingParameters.InitialLocation.Z, CurrentMantlingParameters.InitialAnimationLocation.Z, ZCorrectionAlpha);
				
	FVector NewLocation = FMath::Lerp(CorrectedInitialLocation, CurrentMantlingParameters.TargetLocation, PositionAlpha);
	FRotator NewRotation = FMath::Lerp(CurrentMantlingParameters.InitialRotation, CurrentMantlingParameters.TargetRotation, PositionAlpha);
				
	FVector MeshLocation = CurrentMantlingParameters.TargetMesh->GetComponentLocation();
	FVector DeltaMeshLocation = MeshLocation - CurrentMantlingParameters.InitialTargetMeshLocation;
				
	FVector Delta = DeltaMeshLocation + (NewLocation - GetActorLocation());
	Velocity = Delta / DeltaTime;

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, NewRotation, false, Hit);
}

void UBaseCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if(GetBaseCharacterOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}
	
	switch (CustomMovementMode)
	{
	case (uint8)ECustomMovementMode::CMOVE_Mantling:
	{
		PhysMantling(DeltaTime, Iterations);
		break;
	}
	case (uint8)ECustomMovementMode::CMOVE_Ladder:
	{
		PhysLadder(DeltaTime, Iterations);
		break;
	}
	case (uint8)ECustomMovementMode::CMOVE_Zipline:
	{
		PhysZipline(DeltaTime, Iterations);
		break;
	}
	case (uint8)ECustomMovementMode::CMOVE_WallRun:
	{
		PhysWallRun(DeltaTime, Iterations);
		break;
	}
	case (uint8)ECustomMovementMode::CMOVE_Slide:
	{
		PhysSliding(DeltaTime, Iterations);
		break;
	}
	default:
		break;
	}
	
	Super::PhysCustom(DeltaTime, Iterations);
}

//Movement Modes
void UBaseCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	
	if(MovementMode == MOVE_Swimming)
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(SwimmingCapsuleRadius, SwimmingCapsuleHalfHeight);
	}
	else if (PreviousMovementMode == MOVE_Swimming)
	{
		const ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
															  DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
	}
	
	if (MovementMode == MOVE_Custom)
	{
		switch (CustomMovementMode)
		{
		case (uint8)ECustomMovementMode::CMOVE_Mantling:
			{
				GetWorld()->GetTimerManager().SetTimer(MantlingTimer, this, &UBaseCharacterMovementComponent::EndMantle, CurrentMantlingParameters.Duration, false);
				break;
			}
		default:
			break;
			
		}
	}
	
	if(PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
	{
		CurrentLadder = nullptr;
	}

	if(PreviousCustomMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Zipline)
	{
		CurrentZipline = nullptr;
	}
}

//FSavedMove_GC

void FSavedMove_GC::Clear()
{
	Super::Clear();
	bSavedIsSprinting = 0;
	bSavedIsMantling = 0;
	bSavedIsSliding = 0;
}

uint8 FSavedMove_GC::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	
	// FLAG_Reserved_1		= 0x04,	// Reserved for future use
	// FLAG_Reserved_2		= 0x08,	// Reserved for future use
	// // Remaining bit masks are available for custom flags.
	// FLAG_Custom_0		= 0x10, - Sprinting flag
	// FLAG_Custom_1		= 0x20, - Mantling flag
	// FLAG_Custom_2		= 0x40, = Sliding flag
	// FLAG_Custom_3		= 0x80,
	
	if(bSavedIsSprinting)
	{
		Result |= FLAG_Custom_0;
	}

	if(bSavedIsMantling)
	{
		Result &= ~FLAG_JumpPressed;
		Result |= FLAG_Custom_1;
	}

	if(bSavedIsSliding)
	{
		Result |= FLAG_Custom_2;
	}
	
	return Result;
}

bool FSavedMove_GC::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_GC* NewMove = StaticCast<const FSavedMove_GC*>(NewMovePtr.Get());
	if(bSavedIsSprinting != NewMove->bSavedIsSprinting)
	{
		return false;
	}

	if(bSavedIsMantling != NewMove->bSavedIsMantling)
	{
		return false;
	}

	if(bSavedIsSliding != NewMove->bSavedIsSliding)
	{
		return false;
	}
	
	return Super::CanCombineWith(NewMovePtr, InCharacter, MaxDelta);
}

void FSavedMove_GC::SetMoveFor(ACharacter* InCharacter, float InDeltaTime, FVector const& NewAccel,
                               FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(InCharacter, InDeltaTime, NewAccel, ClientData);

	check(InCharacter->IsA<ABaseCharacter>());
	ABaseCharacter* InBaseCharacter = StaticCast<ABaseCharacter*>(InCharacter);
	UBaseCharacterMovementComponent* MovementComponent = InBaseCharacter->GetBaseCharacterMovementComponent();

	bSavedIsSprinting = MovementComponent->bIsSprinting;
	bSavedIsMantling = InBaseCharacter->bIsMantling;
	bSavedIsSliding = InBaseCharacter->bIsSliding;
}

void FSavedMove_GC::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);
	
	UBaseCharacterMovementComponent* MovementComponent = StaticCast<UBaseCharacterMovementComponent*>(Character->GetMovementComponent());

	MovementComponent->bIsSprinting = bSavedIsSprinting;
}

FNetworkPredictionData_Client_Character_GC::FNetworkPredictionData_Client_Character_GC(
	const UCharacterMovementComponent& ClientMovement)
		: Super(ClientMovement)
{
	
}

FSavedMovePtr FNetworkPredictionData_Client_Character_GC::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_GC());
}
