// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAttributesComponent.h"

#include "Kismet/GameplayStatics.h"
#include "MyProject/Characters/BaseCharacter.h"
#include "MyProject/Components/MovementComponents/BaseCharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCharacterAttributesComponent::UCharacterAttributesComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCharacterAttributesComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharacterAttributesComponent, Health);
}


float UCharacterAttributesComponent::GetHealthPercent() const
{
	return Health / MaxHealth;
}

float UCharacterAttributesComponent::GetStaminaPercent() const
{
	return Stamina / MaxStamina;
}

float UCharacterAttributesComponent::GetOxygenPercent() const
{
	return Oxygen / MaxOxygen;
}

void UCharacterAttributesComponent::AddHealth(float HealthToAdd)
{
	Health = FMath::Clamp(Health + HealthToAdd, 0.f, MaxHealth);
	OnHealthChanged();
}

void UCharacterAttributesComponent::AddStamina()
{
	Stamina = MaxStamina;
	OnStaminaChanged();
}

// Called when the game starts
void UCharacterAttributesComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(MaxHealth > 0.f, TEXT("UCharacterAttributesComponent::BeginPlay() max health can't be equal to 0"))
	checkf(GetOwner()->IsA<ABaseCharacter>(), TEXT("UCharacterAttributeComponent::BeginPlay UCharacterAttributeComponent can bes used only with ABaseCharacter"))
	CachedBaseCharacterOwner = StaticCast<ABaseCharacter*>(GetOwner());
	
	Health = MaxHealth;
	Stamina = MaxStamina;
	Oxygen = MaxOxygen;

	if(GetOwner()->HasAuthority())
	{
		CachedBaseCharacterOwner->OnTakeAnyDamage.AddDynamic(this, &UCharacterAttributesComponent::OnTakeAnyDamage);
	}
	
}


void UCharacterAttributesComponent::OnRep_Health()
{
	OnHealthChanged();
}

void UCharacterAttributesComponent::OnHealthChanged()
{
	if(OnHealthChangedEvent.IsBound())
	{
		OnHealthChangedEvent.Broadcast(GetHealthPercent());
	}
	if(Health <= 0.f)
	{
		// UE_LOG(LogDamage, Warning, TEXT("ABaseCharacter::TakeDamage character %s is killed by an actor %s"), *CachedBaseCharacterOwner->GetName(), *DamageCauser->GetName());
		if(OnDeathEvent.IsBound())
		{
			OnDeathEvent.Broadcast();
		}
	}
}

void UCharacterAttributesComponent::OnStaminaChanged()
{
	if(Stamina <= 0.f)
	{	
		OutOfStaminaEvent.Broadcast(true);
	}
	else if(Stamina >= 50.f)
	{
		OutOfStaminaEvent.Broadcast(false);
	}
}

void UCharacterAttributesComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                                    AController* InstigatedBy, AActor* DamageCauser)
{
	if(!IsAlive())
	{
		return;
	}
	
	UE_LOG(LogDamage, Warning, TEXT("ABaseCharacter::TakeDamage %s resieved %.2f amount of damage from %s"), *CachedBaseCharacterOwner->GetName(), Damage, *DamageCauser->GetName());
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	OnHealthChanged();
}

void UCharacterAttributesComponent::UpdateStaminaValue(float DeltaSeconds)
{
	const float StaminaDelta =  CachedBaseCharacterOwner->GetBaseCharacterMovementComponent()->IsSprinting() ? -SprintStaminaConsumptionVelocity : StaminaRestoreVelocity;
	Stamina += StaminaDelta * DeltaSeconds;
	Stamina = FMath::Clamp(Stamina, 0.f, MaxStamina);
	OnStaminaChanged();
}

void UCharacterAttributesComponent::UpdateOxygenValue(float DeltaSeconds)
{
	const float OxygenDelta = CachedBaseCharacterOwner->IsSwimmingUnderWater() ? -SwimOxygenConsumptionVelocity : OxygenRestoreVelocity;
	Oxygen += OxygenDelta * DeltaSeconds;
	Oxygen = FMath::Clamp(Oxygen, 0.f, MaxOxygen);
	if(Oxygen <= 0.f)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Black, TEXT("Lack Of Oxygen!"));
	}
}

// Called every frame
void UCharacterAttributesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateStaminaValue(DeltaTime);
	UpdateOxygenValue(DeltaTime);
}

