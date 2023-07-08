// Fill out your copyright notice in the Description page of Project Settings.


#include "TurretAttributesComponent.h"

#include "AI/Characters/Turret.h"

UTurretAttributesComponent::UTurretAttributesComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTurretAttributesComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<ATurret>(), TEXT("UCharacterAttributeComponent::BeginPlay UCharacterAttributeComponent can bes used only with ABaseCharacter"))
	CachedTurret = StaticCast<ATurret*>(GetOwner());
	
	Health = MaxHealth;
	CachedTurret->OnTakeAnyDamage.AddDynamic(this, &UTurretAttributesComponent::OnTakeAnyDamage);
}

void UTurretAttributesComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	UE_LOG(LogDamage, Warning, TEXT("ATurret::TakeDamage %s resieved %.2f amount of damage from %s"), *CachedTurret->GetName(), Damage, *DamageCauser->GetName());
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	
	if(Health <= 0.f)
	{
		UE_LOG(LogDamage, Warning, TEXT("ATurret::TakeDamage character %s is destroyed by an actor %s"), *CachedTurret->GetName(), *DamageCauser->GetName());
		if(OnDestroyEvent.IsBound())
		{
			OnDestroyEvent.Broadcast();
		}
	}
}

void UTurretAttributesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
}

