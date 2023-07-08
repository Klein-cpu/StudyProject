// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectiles/Projectile.h"
#include "ExplosiveProjectile.generated.h"

/**
 * 
 */

class UExplosionComponent;
UCLASS()
class MYPROJECT_API AExplosiveProjectile : public AProjectile
{
	GENERATED_BODY()

public:
	AExplosiveProjectile();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UExplosionComponent* ExplosionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float DetonationTime = 2.f;

	virtual void OnProjectileLaunched() override;
	
private:
	void OnDetonationTimerElapsed();

	AController* GetController() const;
	
	FTimerHandle DetonationTimer;
	
};
