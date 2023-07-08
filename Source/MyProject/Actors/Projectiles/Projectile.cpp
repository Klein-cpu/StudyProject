// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/Projectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


AProjectile::AProjectile()
{
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(5.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	SetRootComponent(CollisionComponent);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 2000.f;
	ProjectileMovementComponent->bAutoActivate = false;

	SetReplicates(true);
	AActor::SetReplicateMovement(true);
}

void AProjectile::LaunchProjectile(FVector Direction)
{
	ProjectileMovementComponent->Velocity = Direction * ProjectileMovementComponent->InitialSpeed;
	CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
	OnProjectileLaunched();
}

void AProjectile::SetProjectileActive_Implementation(bool bIsProjectileActive)
{
	ProjectileMovementComponent->SetActive(bIsProjectileActive);
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnCollisionHit);
}

void AProjectile::OnProjectileLaunched()
{
	
}

void AProjectile::OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if(OnProjectileHit.IsBound())
	{
		OnProjectileHit.Broadcast(this, Hit, ProjectileMovementComponent->Velocity.GetSafeNormal());
	}
}

