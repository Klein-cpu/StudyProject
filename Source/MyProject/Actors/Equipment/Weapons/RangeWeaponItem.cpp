// Fill out your copyright notice in the Description page of Project Settings.

#include "RangeWeaponItem.h"

#include "GameCodeTypes.h"
#include "Characters/BaseCharacter.h"
#include "Components/Weapon/WeaponBarrelComponent.h"
#include "Net/UnrealNetwork.h"

ARangeWeaponItem::ARangeWeaponItem()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	WeaponBarrel = CreateDefaultSubobject<UWeaponBarrelComponent>(TEXT("WeaponBarrel"));
	WeaponBarrel->SetupAttachment(WeaponMesh, SocketWeaponMuzzle);

	ReticleType = EReticleType::Default;
	
	EquippedSocketName = SocketCharacterWeapon;
}

void ARangeWeaponItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARangeWeaponItem, Ammo);
}

void ARangeWeaponItem::StartFire()
{
	if(GetWorld()->GetTimerManager().IsTimerActive(ShotTimer))
	{
		return;
	}
	
	bIsFiring = true;
	MakeShot();
}

void ARangeWeaponItem::StopFire()
{
	bIsFiring = false;
}

bool ARangeWeaponItem::IsFiring() const
{
	return bIsFiring;
}

void ARangeWeaponItem::StartAim()
{
	bIsAiming = true;
}

void ARangeWeaponItem::StopAim()
{
	bIsAiming = false;
}

void ARangeWeaponItem::Server_StartReload_Implementation()
{
	Multicast_Reload();
}

void ARangeWeaponItem::Multicast_Reload_Implementation()
{
	StartReload();
}

void ARangeWeaponItem::StartReload()
{
	if(bIsReloading)
	{
		return;
	}
	
	ABaseCharacter* CharacterOwner = GetCharacterOwner();
	if(!IsValid(CharacterOwner))
	{
		return;
	}
	
	bIsReloading = true;
	if(IsValid(CharacterReloadMontage))
	{
		float MontageDuration = CharacterOwner->PlayAnimMontage(CharacterReloadMontage);
		PlayAnimMontage(WeaponReloadMontage);
		if(ReloadType == EReloadType::FullClip)
		{
			GetWorld()->GetTimerManager().SetTimer(ReloadTimer,  [this](){ EndReload(true); }, MontageDuration, false);
		}
	}
	else
	{
		EndReload(true);
	}
}

void ARangeWeaponItem::EndReload(bool bIsSuccess)
{
	if(!bIsReloading)
	{
		return;
	}

	ABaseCharacter* CharacterOwner = GetCharacterOwner();
	
	if(!bIsSuccess)
	{
        if(!IsValid(CharacterOwner))
        {
        	return;
        }
		CharacterOwner->StopAnimMontage(CharacterReloadMontage);
		StopAnimMontage(WeaponReloadMontage);
	}

	if(ReloadType == EReloadType::ByBullet)
	{
		UAnimInstance* CharacterAnimInstance = IsValid(CharacterOwner) ? CharacterOwner->GetMesh()->GetAnimInstance() : nullptr;
		if(IsValid(CharacterAnimInstance))
		{
			CharacterAnimInstance->Montage_JumpToSection(SectionMontageReloadEnd, CharacterReloadMontage);
		}

		UAnimInstance* WeaponAnimInstance = WeaponMesh->GetAnimInstance();
		if(IsValid(WeaponAnimInstance))
		{
			WeaponAnimInstance->Montage_JumpToSection(SectionMontageReloadEnd, WeaponReloadMontage);
		}
	}
	
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
	
	bIsReloading = false;
	if(bIsSuccess && OnReloadComplete.IsBound())
	{
		OnReloadComplete.Broadcast();
	}
}

void ARangeWeaponItem::ChangeWeaponRegime()
{
	if(CurrentWeaponRegime.structNum == PrimaryWeaponRegime.structNum)
	{
		CurrentWeaponRegime = SecondaryWeaponRegime;
	}
	else
	{
		CurrentWeaponRegime = PrimaryWeaponRegime;
	}
	WeaponBarrel->ChangeWeaponBarrelRegime();
	
	Ammo = Ammo + CachedAmmo;
	CachedAmmo = Ammo - CachedAmmo;
	Ammo = Ammo - CachedAmmo;
	
	if(OnWeaponRegimeChanged.IsBound())
	{
		OnWeaponRegimeChanged.Broadcast();
	}
}

bool ARangeWeaponItem::IsReloading() const
{
	return bIsReloading;
}

int32 ARangeWeaponItem::GetAmmo() const
{
	return Ammo;
}

int32 ARangeWeaponItem::GetMaxAmmo() const
{
	return CurrentWeaponRegime.MaxAmmo;
}

void ARangeWeaponItem::AddAmmo(int32 NewAmmo)
{
	Ammo += NewAmmo;
	if(OnAmmoChanged.IsBound())
	{
		OnAmmoChanged.Broadcast(Ammo);
	}
}

EAmunitionType ARangeWeaponItem::GetAmmoType() const
{
	return CurrentWeaponRegime.AmmoType;
}

bool ARangeWeaponItem::CanShoot() const
{
	return Ammo > 0.f;
}

float ARangeWeaponItem::GetAimFOV() const
{
	return AimFOV;
}

float ARangeWeaponItem::GetAimMovementMaxSpeed() const
{
	return AimMovementMaxSpeed;
}

float ARangeWeaponItem::GetAimTurnModifier() const
{
	return AimTurnModifier;
}

float ARangeWeaponItem::GetAimLookUpModifier() const
{
	return AimLookUpModifier;
}

FTransform ARangeWeaponItem::GetForeGripTransform() const
{
	return WeaponMesh->GetSocketTransform(SocketWeaponForeGrip);
}

EReticleType ARangeWeaponItem::GetReticleType() const
{
	return bIsAiming ? AimReticleType : ReticleType;
}

void ARangeWeaponItem::BeginPlay()
{
	Super::BeginPlay();

	PrimaryWeaponRegime.structNum = 1;
	SecondaryWeaponRegime.structNum = 2;
	
	CurrentWeaponRegime = PrimaryWeaponRegime;
	AddAmmo(CurrentWeaponRegime.MaxAmmo);
	CachedAmmo = SecondaryWeaponRegime.MaxAmmo;
}

float ARangeWeaponItem::GetCurrentBulletSpreadAngle() const
{
	float AngleInDegrees = bIsAiming ? AimSpreadAngle : SpreadAngle;
	return FMath::DegreesToRadians(AngleInDegrees);
}

void ARangeWeaponItem::MakeShot()
{
	ABaseCharacter* CharacterOwner = GetCharacterOwner();
	
	if(!IsValid(CharacterOwner))
	{
		return;
	}

	if(!CanShoot())
	{
		StopFire();
		if(Ammo == 0 && bAutoReload)
		{
			CharacterOwner->Reload();
		}
		return;
	}

	EndReload(false);
	
	CharacterOwner->PlayAnimMontage(CharacterFireMontage);
	PlayAnimMontage(WeaponFireMontage);

	FVector ShotLocation;
	FRotator ShotRotation;
	
	if(CharacterOwner->IsPlayerControlled())
	{
		APlayerController* Controller = CharacterOwner->GetController<APlayerController>();
		Controller->GetPlayerViewPoint(ShotLocation, ShotRotation);
	}
	else
	{
		ShotLocation = WeaponBarrel->GetComponentLocation();
		ShotRotation = CharacterOwner->GetBaseAimRotation();
	}

	const FVector ShotDirection = ShotRotation.RotateVector(FVector::ForwardVector);

	AddAmmo(-1);
	WeaponBarrel->Shot(ShotLocation, ShotDirection, GetCurrentBulletSpreadAngle());
	
	GetWorld()->GetTimerManager().SetTimer(ShotTimer, this, &ARangeWeaponItem::OnShotTimerElapsed, GetShotTimerInterval(),
											   false);
}

void ARangeWeaponItem::OnShotTimerElapsed()
{
	if(!bIsFiring)
	{
		return;
	}

	switch (CurrentWeaponRegime.WeaponFireMode)
	{
	case EWeaponFireMode::Single:
		{
			StopFire();
			break;
		}
	case EWeaponFireMode::FullAuto:
		{
			MakeShot();
			break;
		}
	}
}

float ARangeWeaponItem::GetShotTimerInterval() const
{
	return 60.f / CurrentWeaponRegime.FireRate;
}

float ARangeWeaponItem::PlayAnimMontage(UAnimMontage* AnimMontage)
{
	UAnimInstance* WeaponAnimInstance = WeaponMesh->GetAnimInstance();
	float Result = 0.f;
	if (IsValid(WeaponAnimInstance))
	{
		Result = WeaponAnimInstance->Montage_Play(AnimMontage);
	}
	return Result;
}

void ARangeWeaponItem::StopAnimMontage(UAnimMontage* AnimMontage, float BlendOutTime)
{
	UAnimInstance* WeaponAnimInstance = WeaponMesh->GetAnimInstance();
	if(IsValid(WeaponAnimInstance))
	{
		WeaponAnimInstance->Montage_Stop(BlendOutTime, AnimMontage);
	}
}
