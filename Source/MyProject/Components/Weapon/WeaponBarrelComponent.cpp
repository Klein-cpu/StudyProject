// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBarrelComponent.h"

#include "GameCodeTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/DebugSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Actors/Projectiles/Projectile.h"
#include "Components/DecalComponent.h"
#include "Net/UnrealNetwork.h"

UWeaponBarrelComponent::UWeaponBarrelComponent()
{
	SetIsReplicatedByDefault(true);
}

void UWeaponBarrelComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams RepParams;
	RepParams.Condition = COND_SimulatedOnly;
	RepParams.RepNotifyCondition = REPNOTIFY_Always;
	DOREPLIFETIME_WITH_PARAMS_FAST(UWeaponBarrelComponent, LastShotsInfo, RepParams);
	DOREPLIFETIME(UWeaponBarrelComponent, ProjectilePool);
	DOREPLIFETIME(UWeaponBarrelComponent, CurrentProjectileIndex);
}

void UWeaponBarrelComponent::Shot(FVector ShotStart, FVector ShotDirection, float SpreadAngle)
{
	TArray<FShotInfo> ShotsInfo;
	for (int i = 0; i < CurrentWeaponBarrelRegime.BulletsPerShot; i++)
	{
		ShotDirection += GetBulletSpreadOffset(FMath::RandRange(0.f, SpreadAngle),
		                                       ShotDirection.ToOrientationRotator());
		ShotDirection = ShotDirection.GetSafeNormal();
		ShotsInfo.Emplace(ShotStart, ShotDirection);
	}
	if(GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_Shot(ShotsInfo);
	}
	ShotInternal(ShotsInfo);
}

void UWeaponBarrelComponent::ChangeWeaponBarrelRegime()
{
	if(CurrentWeaponBarrelRegime.structNum == PrimaryWeaponRegime.structNum)
	{
		CurrentWeaponBarrelRegime = SecondaryWeaponRegime;
	}
	else
	{
		CurrentWeaponBarrelRegime = PrimaryWeaponRegime;
	}
}

void UWeaponBarrelComponent::BeginPlay()
{
	Super::BeginPlay();

	PrimaryWeaponRegime.structNum = 1;
	SecondaryWeaponRegime.structNum = 2;
	CurrentWeaponBarrelRegime = PrimaryWeaponRegime;
	
	if(GetOwnerRole() < ROLE_Authority)
	{
		return;
	}
	if(!ProjectileClass)
	{
		return;
	}
	
	ProjectilePool.Reserve(ProjectilePoolSize);
	for(int32 i = 0; i < ProjectilePoolSize; i++)
	{
		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, ProjectilePoolLocation, FRotator::ZeroRotator);
		Projectile->SetOwner(GetOwningPawn());
		Projectile->SetProjectileActive(false);
		ProjectilePool.Add(Projectile);
	}
}

bool UWeaponBarrelComponent::HitScan(FVector ShotStart, FVector& ShotEnd, FVector ShotDirection)
{
	FHitResult ShotResult;
	bool bHasHit = GetWorld()->LineTraceSingleByChannel(ShotResult, ShotStart, ShotEnd, ECC_BULLET);
	if (bHasHit)
	{
		ShotEnd = ShotResult.ImpactPoint;
		ProcessHit(ShotResult, ShotDirection);
	}
	return bHasHit;
}

FVector UWeaponBarrelComponent::GetBulletSpreadOffset(float Angle, FRotator ShotRotation) const
{
	float SpreadSize = FMath::Tan(Angle);
	float RotationAngle = FMath::RandRange(0.f, 2 * PI);

	float SpreadY = FMath::Cos(RotationAngle);
	float SpreadZ = FMath::Sin(RotationAngle);

	FVector Result = (ShotRotation.RotateVector(FVector::UpVector) * SpreadZ + ShotRotation.
		RotateVector(FVector::RightVector) * SpreadY) * SpreadSize;
	return Result;
}

void UWeaponBarrelComponent::LaunchProjectile(const FVector& LaunchStart, const FVector& LaunchDirection)
{
	AProjectile* Projectile = ProjectilePool[CurrentProjectileIndex];

	if(IsValid(Projectile))
	{
		Projectile->SetActorLocation(LaunchStart);
		Projectile->SetActorRotation(LaunchDirection.ToOrientationRotator());
		Projectile->SetProjectileActive(true);
		Projectile->OnProjectileHit.AddDynamic(this, &UWeaponBarrelComponent::ProcessProjectileHit);
		Projectile->LaunchProjectile(LaunchDirection.GetSafeNormal());
		++CurrentProjectileIndex;
		if(CurrentProjectileIndex == ProjectilePool.Num())
		{
			CurrentProjectileIndex = 0;
		}
	}
}

void UWeaponBarrelComponent::ShotInternal(const TArray<FShotInfo>& ShotsInfo)
{
	if(GetOwner()->HasAuthority())
	{
		LastShotsInfo = ShotsInfo;
	}
	
	FVector MuzzleLocation = GetComponentLocation();
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashFX, MuzzleLocation,
												   GetComponentRotation());
	for (const auto& ShotInfo : ShotsInfo)
	{
		FVector ShotStart = ShotInfo.GetLocation();
		FVector ShotDirection = ShotInfo.GetDirection();
		FVector ShotEnd = ShotStart + FiringRange * ShotDirection;

#if ENABLE_DRAW_DEBUG
		UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
		bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryRangeWeapon);
#else
		bool bIsDebugEnabled = false;
#endif

		switch (CurrentWeaponBarrelRegime.HitRegistration)
		{
		case EHitRegistrationType::HitScan:
			{
				bool bHasHit = HitScan(ShotStart, ShotEnd, ShotDirection);
				if (bIsDebugEnabled && bHasHit)
				{
					DrawDebugSphere(GetWorld(), ShotEnd, 10.f, 24, FColor::Red, false, 1.f);
				}
				break;
			}
		case EHitRegistrationType::Projectile:
			{
				LaunchProjectile(ShotStart, ShotDirection);
				break;
			}
		}

		UNiagaraComponent* TraceFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), TraceFX, MuzzleLocation, GetComponentRotation());
		if(IsValid(TraceFXComponent))
		{
			TraceFXComponent->SetVectorParameter(FXParamTraceEnd, ShotEnd);
		}

		if (bIsDebugEnabled)
		{
			DrawDebugLine(GetWorld(), MuzzleLocation, ShotEnd, FColor::Red, false, 1.f, 0, 3.f);
		}
	}
}

void UWeaponBarrelComponent::OnRep_LastShotsInfo()
{
	ShotInternal(LastShotsInfo);
}

void UWeaponBarrelComponent::ProcessHit(const FHitResult& HitResult, const FVector& Direction)
{
	AActor* HitActor = HitResult.GetActor();
	if (GetOwnerRole() == ROLE_Authority && IsValid(HitActor))
	{
		FPointDamageEvent DamageEvent;
		DamageEvent.HitInfo = HitResult;
		DamageEvent.ShotDirection = Direction;
		DamageEvent.DamageTypeClass = CurrentWeaponBarrelRegime.DamageTypeClass;
				
		if(IsValid(DamageCurve))
		{
			DamageMultiplier = DamageCurve->GetFloatValue(FVector::Distance(GetComponentLocation(), HitResult.ImpactPoint));
		}
		HitActor->TakeDamage(CurrentWeaponBarrelRegime.DamageAmount * DamageMultiplier, DamageEvent, GetController(), GetOwner());
	}
	
	UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(), DefaultDecalInfo.DecalMaterial, DefaultDecalInfo.DecalSize, HitResult.ImpactPoint,
		HitResult.ImpactNormal.ToOrientationRotator());
	
	if (IsValid(DecalComponent))
	{
		DecalComponent->SetFadeScreenSize(.0001f);
		DecalComponent->SetFadeOut(DefaultDecalInfo.DecalLifeTime, DefaultDecalInfo.DecalFadeOutTime);
	}
}

void UWeaponBarrelComponent::ProcessProjectileHit(AProjectile* Projectile, const FHitResult& HitResult,
	const FVector& Direction)
{
	Projectile->SetProjectileActive(false);
	Projectile->SetActorLocation(ProjectilePoolLocation);
	Projectile->SetActorRotation(FRotator::ZeroRotator);
	Projectile->OnProjectileHit.RemoveAll(this);
	ProcessHit(HitResult, Direction);
}

APawn* UWeaponBarrelComponent::GetOwningPawn() const
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if(!IsValid(PawnOwner))
	{
		PawnOwner = Cast<APawn>(GetOwner()->GetOwner());
	}
	return PawnOwner;
}

AController* UWeaponBarrelComponent::GetController() const
{
	const APawn* PawnOwner = GetOwningPawn();
	
	return IsValid(PawnOwner) ? PawnOwner->GetController() : nullptr;
}

void UWeaponBarrelComponent::Server_Shot_Implementation(const TArray<FShotInfo>& ShotsInfo)
{
		ShotInternal(ShotsInfo);
}
