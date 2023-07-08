// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCodeTypes.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Pawn.h"
#include "Turret.generated.h"

class UTurretAttributesComponent;
class UWeaponBarrelComponent;
UENUM(BlueprintType)
enum class ETurretState : uint8
{
	Searching,
	Firing
};

UCLASS()
class MYPROJECT_API ATurret : public APawn, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ATurret();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void Tick(float DeltaTime) override;

	virtual void PossessedBy(AController* NewController) override;
	
	void OnCurrentTargetSet();
	
	virtual FVector GetPawnViewLocation() const override;
	virtual FRotator GetViewRotation() const override;
	
	UPROPERTY(ReplicatedUsing=OnRep_CurrentTarget)
	AActor* CurrentTarget = nullptr;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret | Components")
	UTurretAttributesComponent* TurretAttributesComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret | Components")
	USceneComponent* TurretBaseComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret | Components")
	USceneComponent* TurretBarrelComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret | Components")
	UWeaponBarrelComponent* WeaponBarrel;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BaseSearchingRotationRate = 60.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BarrelPitchRotationRate = 60.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BaseTrackingInterpSpeed = -30.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxBarrelPitchAngle = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MinBarrelPitchAngle = -30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters | Fire", meta = (ClampMin = 1.f, UIMin = 1.f))
	float FireRate = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters | Fire", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BulletSpreadAngle = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters | Fire", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BeginFireDelay = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters | Team")
	ETeams Team = ETeams::Enemy;
	
private:
	void SearchingMovement(float DeltaTime);
	void TrackingMovement(float DeltaTime);
	
	void SetCurrentTurretState(ETurretState NewState);
	
	ETurretState CurrentTurretState = ETurretState::Searching;

	UFUNCTION()
	void OnRep_CurrentTarget();

	float GetFireInterval() const;
	void MakeShot();
	
	FTimerHandle ShotTimer;
};
