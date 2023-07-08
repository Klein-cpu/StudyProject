// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/ExplosiveProjectile.h"

#include "ExplosionComponent.h"

AExplosiveProjectile::AExplosiveProjectile()
{
	ExplosionComponent = CreateDefaultSubobject<UExplosionComponent>(TEXT("ExplosionComponent"));
	ExplosionComponent->SetupAttachment(GetRootComponent());
}

void AExplosiveProjectile::OnProjectileLaunched()
{
	Super::OnProjectileLaunched();
	GetWorld()->GetTimerManager().SetTimer(DetonationTimer, this, &AExplosiveProjectile::OnDetonationTimerElapsed, DetonationTime, false);
}

void AExplosiveProjectile::OnDetonationTimerElapsed()
{
	ExplosionComponent->Multicast_Explode(GetController());
}

AController* AExplosiveProjectile::GetController() const
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	
	return IsValid(PawnOwner) ? PawnOwner->GetController() : nullptr;
}
