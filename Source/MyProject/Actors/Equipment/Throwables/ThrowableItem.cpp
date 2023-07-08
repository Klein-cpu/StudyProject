// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableItem.h"

#include "Actors/Projectiles/Projectile.h"
#include "Characters/BaseCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"

void AThrowableItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AThrowableItem, AvailableItems);
}

void AThrowableItem::Throw(AProjectile* Projectile)
{
	ABaseCharacter* CharacterOwner = GetCharacterOwner();
	if(!IsValid(CharacterOwner))
	{
		return;
	}

	AController* Controller = CharacterOwner->GetController<AController>();
	if (!IsValid(Controller))
	{
		return;
	}
	
	FVector PlayerViewPoint;
	FRotator PlayerViewRotation;
	
	Controller->GetPlayerViewPoint(PlayerViewPoint, PlayerViewRotation);
	FTransform PlayerViewTransform(PlayerViewRotation, PlayerViewPoint);

	FVector PlayerViewDirection = PlayerViewRotation.RotateVector(FVector::ForwardVector);
	FVector PlayerViewUpVector = PlayerViewRotation.RotateVector(FVector::UpVector);

	FVector LaunchDirection = PlayerViewDirection + FMath::Tan(FMath::DegreesToRadians(ThrowAngle)) * PlayerViewUpVector;

	FVector ThrowableSocketLocation = CharacterOwner->GetMesh()->GetSocketLocation(SocketCharacterThrowable);
	FVector ThrowableLocationInViewSpace = PlayerViewTransform.InverseTransformPosition(ThrowableSocketLocation);
	
	FVector SpawnLocation = PlayerViewPoint + PlayerViewDirection * ThrowableLocationInViewSpace.X;
	
	Projectile->SetActorLocation(SpawnLocation);
	Projectile->SetActorRotation(FRotator::ZeroRotator);

	AddItems(-1);
	if(IsValid(Projectile))
	{
		Projectile->SetOwner(GetOwner());
		Projectile->LaunchProjectile(LaunchDirection.GetSafeNormal());
	}
	
}

int32 AThrowableItem::GetAmountOfItems()
{
	return AvailableItems;
}

void AThrowableItem::AddItems(int32 Amount)
{
	AvailableItems += Amount;
}

EAmunitionType AThrowableItem::GetThrowableItemType()
{
	return ThrowableItemType;
}

void AThrowableItem::BeginPlay()
{
	Super::BeginPlay();
}
