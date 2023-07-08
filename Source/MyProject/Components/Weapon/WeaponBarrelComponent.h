// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WeaponBarrelComponent.generated.h"

UENUM(BlueprintType)
enum class EHitRegistrationType : uint8
{
	HitScan,
	Projectile
};

USTRUCT(BlueprintType)
struct FWeaponBarrelRegime
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit registration")
	EHitRegistrationType HitRegistration = EHitRegistrationType::HitScan;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 1, UIMin = 1))
	int32 BulletsPerShot = 1.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UDamageType> DamageTypeClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float DamageAmount = 20.f;
	
	int32 structNum;
};

USTRUCT(BlueprintType)
struct FDecalInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal info")
	UMaterialInterface* DecalMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal info")
	FVector DecalSize = FVector(5.f, 5.f, 5.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal info")
	float DecalLifeTime = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal info")
	float DecalFadeOutTime = 5.f;
	
};

USTRUCT(BlueprintType)
struct FShotInfo
{
	GENERATED_BODY()

	FShotInfo() : Location_Mul_10(FVector_NetQuantize100::ZeroVector), Direction(FVector_NetQuantizeNormal::ZeroVector) {};
	FShotInfo(FVector Location, FVector Direction) : Location_Mul_10(Location * 10.f), Direction(Direction) {};
	
	UPROPERTY()
	FVector_NetQuantize100 Location_Mul_10;

	UPROPERTY()
	FVector_NetQuantizeNormal Direction;

	FVector GetLocation() const { return Location_Mul_10 * 0.1f; }
	FVector GetDirection() const { return Direction; }
};

class AProjectile;
class UNiagaraSystem;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYPROJECT_API UWeaponBarrelComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UWeaponBarrelComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void Shot(FVector ShotStart, FVector ShotDirection, float SpreadAngle);
	
	void ChangeWeaponBarrelRegime();
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barrel attributes")
	FWeaponBarrelRegime PrimaryWeaponRegime;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barrel attributes")
	FWeaponBarrelRegime SecondaryWeaponRegime;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barrel attributes")
	float FiringRange = 5000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barrel attributes | Hit registration", meta = (EditCondition = "HitRegistration == EHitRegistrationType::Projectile"))
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barrel attributes | Hit registration", meta = (UIMin = 1, ClampMin = 1, EditCondition = "HitRegistration == EHitRegistrationType::Projectile"))
	int32 ProjectilePoolSize = 10;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barrel attributes | Damage")
	UCurveFloat* DamageCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barrel attributes | VFX")
	UNiagaraSystem* MuzzleFlashFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barrel attributes | VFX")
	UNiagaraSystem* TraceFX;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barrel attributes | Decals")
	FDecalInfo DefaultDecalInfo;

private:
	void ShotInternal(const TArray<FShotInfo>& ShotsInfo);
	
	UFUNCTION(Server, Reliable)
	void Server_Shot(const TArray<FShotInfo>& ShotsInfo);

	UPROPERTY(ReplicatedUsing=OnRep_LastShotsInfo)
	TArray<FShotInfo> LastShotsInfo;

	UPROPERTY(Replicated)
	TArray<AProjectile*> ProjectilePool;

	UPROPERTY(Replicated)
	int32 CurrentProjectileIndex;
	
	UFUNCTION()
	void OnRep_LastShotsInfo();
	
	UFUNCTION()
	void ProcessHit(const FHitResult& HitResult, const FVector& Direction);

	UFUNCTION()
	void ProcessProjectileHit(AProjectile* Projectile, const FHitResult& HitResult, const FVector& Direction);
	
	bool HitScan(FVector ShotStart, OUT FVector& ShotEnd, FVector ShotDirection);
	
	APawn* GetOwningPawn() const;
	AController* GetController() const;
	
	void LaunchProjectile(const FVector& LaunchStart, const FVector& LaunchDirection);
	
	FVector GetBulletSpreadOffset(float Angle, FRotator ShotRotation) const;

	const FVector ProjectilePoolLocation = FVector(0.f,0.f,-100.f);
	
	float DamageMultiplier = 1.f;

	FWeaponBarrelRegime CurrentWeaponBarrelRegime;
	
};
