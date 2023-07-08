// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Equipment/EquipableItem.h"
#include "RangeWeaponItem.generated.h"

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	Single,
	FullAuto
};

UENUM(BlueprintType)
enum class EReloadType : uint8
{
	FullClip,
	ByBullet
};

USTRUCT(BlueprintType)
struct FWeaponRegime
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EWeaponFireMode WeaponFireMode = EWeaponFireMode::Single;
	
	//Rate of fire in rounds per minute
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 1.f, UIMin = 1.f))
	float FireRate = 600.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = 1, UIMin = 1))
	EAmunitionType AmmoType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = 1, UIMin = 1))
	int32 MaxAmmo = 30;

	int32 structNum;
};

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE(FOnReloadComplete)

DECLARE_MULTICAST_DELEGATE(FOnWeaponRegimeChanged)

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAmmoChanged, int32);

UCLASS(Blueprintable)
class MYPROJECT_API ARangeWeaponItem : public AEquipableItem
{
	GENERATED_BODY()

public:
	ARangeWeaponItem();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void StartFire();
	void StopFire();

	bool IsFiring() const;
	
	void StartAim();
	void StopAim();

	UFUNCTION(Server, Reliable)
	void Server_StartReload();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Reload();
	
	void StartReload();
	void EndReload(bool bIsSuccess);

	void ChangeWeaponRegime();
	
	bool IsReloading() const;
	
	int32 GetAmmo() const;
	int32 GetMaxAmmo() const;

	void AddAmmo(int32 NewAmmo);

	EAmunitionType GetAmmoType() const;
	
	bool CanShoot() const;
	
	float GetAimFOV() const;
	float GetAimMovementMaxSpeed() const;
	float GetAimTurnModifier() const;
	float GetAimLookUpModifier() const;
	
	FTransform GetForeGripTransform() const;

	FOnAmmoChanged OnAmmoChanged;
	FOnWeaponRegimeChanged OnWeaponRegimeChanged;
	FOnReloadComplete OnReloadComplete;

	virtual EReticleType GetReticleType() const override;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWeaponBarrelComponent* WeaponBarrel;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Weapon")
	UAnimMontage* WeaponFireMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Weapon")
	UAnimMontage* WeaponReloadMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Character")
	UAnimMontage* CharacterFireMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Character")
	UAnimMontage* CharacterReloadMontage;
	
	//FullClip reload type adds ammo only when the whole animation is success
	//ByBullet reload type requires section "ReloadEnd" in character reload animation 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters")
	EReloadType ReloadType = EReloadType::FullClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters")
	FWeaponRegime PrimaryWeaponRegime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters")
	FWeaponRegime SecondaryWeaponRegime;
	
	//Bullet spread half angle in degrees
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters", meta = (ClampMin = 0.f, UIMin = 0.f, ClampMax = 2.f, UIMax = 2.f))
	float SpreadAngle = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.f, UIMin = 0.f, ClampMax = 2.f, UIMax = 2.f))
	float AimSpreadAngle = .25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float AimMovementMaxSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float AimFOV = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.f, UIMin = 0.f, ClampMax = 1.f, UIMax = 1.f))
	float AimTurnModifier = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.f, UIMin = 0.f, ClampMax = 1.f, UIMax = 1.f))
	float AimLookUpModifier = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Ammo")
	bool bAutoReload = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reticle")
	EReticleType AimReticleType = EReticleType::Default;

private:
	UPROPERTY(Replicated)
	int32 Ammo = 0;
	
	int32 CachedAmmo = 0;
	
	FWeaponRegime CurrentWeaponRegime;
	
	bool bIsReloading = false;
	bool bIsFiring = false;
	
	float GetCurrentBulletSpreadAngle() const;
	
	bool bIsAiming;
	
	void MakeShot();

	void OnShotTimerElapsed();
	float GetShotTimerInterval() const;
	
	float PlayAnimMontage(UAnimMontage* AnimMontage);
	void StopAnimMontage(UAnimMontage* AnimMontage, float BlendOutTime = 0.f);
	
	FTimerHandle ShotTimer;
	FTimerHandle ReloadTimer;
	
};
