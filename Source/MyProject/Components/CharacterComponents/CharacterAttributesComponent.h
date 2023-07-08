// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterAttributesComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float);
DECLARE_MULTICAST_DELEGATE(FOnDeathEventSignature);
DECLARE_MULTICAST_DELEGATE_OneParam(FOutOfStaminaEventSignature, bool bOutOfStamina);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYPROJECT_API UCharacterAttributesComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCharacterAttributesComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	FOnDeathEventSignature OnDeathEvent;
	FOnHealthChanged OnHealthChangedEvent;
	FOutOfStaminaEventSignature OutOfStaminaEvent;
	
	bool IsAlive() const
	{ return Health > 0; }

	float GetHealthPercent() const;
	float GetStaminaPercent() const;
	float GetOxygenPercent() const;

	void AddHealth(float HealthToAdd);
	void AddStamina();
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (UIMin = 0.f))
	float MaxHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (UIMin = 0.f))
	float MaxStamina = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (UIMin = 0.f))
	float StaminaRestoreVelocity = 5.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (UIMin = 0.f))
	float SprintStaminaConsumptionVelocity = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.f))
	float MaxOxygen = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.f))
	float OxygenRestoreVelocity = 15.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.f))
	float SwimOxygenConsumptionVelocity = 2.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (UIMin = 0.f))
	float OxygenLackHealthConsumption = 10.f;
	
private:
	UPROPERTY(ReplicatedUsing=OnRep_Health)
	float Health = 0.f;
	UFUNCTION()
	void OnRep_Health();
	void OnHealthChanged();
	
	float Stamina = 0.f;
	void OnStaminaChanged();
	
	float Oxygen = 0.f;

	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void UpdateStaminaValue(float DeltaSeconds);

	UFUNCTION()
	void UpdateOxygenValue(float DeltaSeconds);
	
	TWeakObjectPtr<class ABaseCharacter> CachedBaseCharacterOwner;
};
