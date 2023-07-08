// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Characters/Turret.h"

#include "AIController.h"
#include "Components/AIComponents/TurretAttributesComponent.h"
#include "Components/Weapon/WeaponBarrelComponent.h"
#include "Net/UnrealNetwork.h"

ATurret::ATurret()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* TurretRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretRoot"));
	SetRootComponent(TurretRoot);

	TurretBaseComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TurretBase"));
	TurretBaseComponent->SetupAttachment(TurretRoot);

	TurretBarrelComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TurretBarrel"));
	TurretBarrelComponent->SetupAttachment(TurretBaseComponent);

	WeaponBarrel = CreateDefaultSubobject<UWeaponBarrelComponent>(TEXT("WeaponBarrel"));
	WeaponBarrel->SetupAttachment(TurretBarrelComponent);
	
	TurretAttributesComponent = CreateDefaultSubobject<UTurretAttributesComponent>(TEXT("TurretAttributes"));

	SetReplicates(true);
}

void ATurret::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATurret, CurrentTarget)
}

void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentTurretState)
	{
	case ETurretState::Searching:
		{
			SearchingMovement(DeltaTime);
			break;
		}
	case ETurretState::Firing:
		{
			TrackingMovement(DeltaTime);
			break;
		}
	}
}

void ATurret::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AAIController* AIController = Cast<AAIController>(NewController);

	if(IsValid(AIController))
	{
		FGenericTeamId TeamId(static_cast<uint8>(Team));
		AIController->SetGenericTeamId(TeamId);
	}
}

void ATurret::OnCurrentTargetSet()
{
	ETurretState NewState = IsValid(CurrentTarget) ? ETurretState::Firing : ETurretState::Searching;
	
	SetCurrentTurretState(NewState);
}

FVector ATurret::GetPawnViewLocation() const
{
	return WeaponBarrel->GetComponentLocation();
}

FRotator ATurret::GetViewRotation() const
{	
	return WeaponBarrel->GetComponentRotation();	
}

void ATurret::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATurret::SearchingMovement(float DeltaTime)
{
	FRotator TurretBaseRotation = TurretBaseComponent->GetRelativeRotation();
	TurretBaseRotation.Yaw += DeltaTime * BaseSearchingRotationRate;
	TurretBaseComponent->SetRelativeRotation(TurretBaseRotation);

	FRotator TurretBarrelRotation = TurretBarrelComponent->GetRelativeRotation();
	TurretBarrelRotation.Pitch = FMath::FInterpTo(TurretBarrelRotation.Pitch, 0.f, DeltaTime, BarrelPitchRotationRate);
	TurretBarrelComponent->SetRelativeRotation(TurretBarrelRotation);
}

void ATurret::TrackingMovement(float DeltaTime)
{
	const FVector BaseLookAtDirection = (CurrentTarget->GetActorLocation() - TurretBaseComponent->GetComponentLocation()).GetSafeNormal2D();
	FQuat LookAtQuat = BaseLookAtDirection.ToOrientationQuat();
	FQuat TargetQuat = FMath::QInterpTo(TurretBaseComponent->GetComponentQuat(), LookAtQuat, DeltaTime, BaseTrackingInterpSpeed);
	TurretBaseComponent->SetWorldRotation(TargetQuat);

	const FVector BarrelLookAtDirection = (CurrentTarget->GetActorLocation() - TurretBarrelComponent->GetComponentLocation()).GetSafeNormal();
	float LookAtPitchAngle = BarrelLookAtDirection.ToOrientationRotator().Pitch;

	FRotator BarrelLocalRotation = TurretBarrelComponent->GetRelativeRotation();
	BarrelLocalRotation.Pitch = FMath::FInterpTo(BarrelLocalRotation.Pitch, LookAtPitchAngle, DeltaTime, BarrelPitchRotationRate);
	TurretBarrelComponent->SetRelativeRotation(BarrelLocalRotation);
	
}

void ATurret::SetCurrentTurretState(ETurretState NewState)
{
	bool bIsStateChanged = NewState != CurrentTurretState;
	CurrentTurretState = NewState;

	if(!bIsStateChanged)
	{
		return;
	}

	switch (CurrentTurretState)
	{
	case ETurretState::Searching:
		{
			GetWorld()->GetTimerManager().ClearTimer(ShotTimer);
			break;
		}
	case ETurretState::Firing:
		{
			GetWorld()->GetTimerManager().SetTimer(ShotTimer, this, &ATurret::MakeShot, GetFireInterval(), true, BeginFireDelay);
			break;
		}
	}
	
}

void ATurret::OnRep_CurrentTarget()
{
	OnCurrentTargetSet();
}

float ATurret::GetFireInterval() const
{
	return 60.f / FireRate;
}

void ATurret::MakeShot()
{
	const FVector ShotLocation = WeaponBarrel->GetComponentLocation();
	const FVector ShotDirection = WeaponBarrel->GetComponentRotation().RotateVector(FVector::ForwardVector);
	const float SpreadAngle = FMath::DegreesToRadians(BulletSpreadAngle);
	WeaponBarrel->Shot(ShotLocation, ShotDirection, SpreadAngle);
}
