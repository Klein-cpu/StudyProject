// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurretAttributesComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDestroyEventSignature);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYPROJECT_API UTurretAttributesComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTurretAttributesComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable)
	FOnDestroyEventSignature OnDestroyEvent;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (UIMin = 0.f))
	float MaxHealth = 100.f;

private:
	float Health = 0.f;

	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	
	TWeakObjectPtr<class ATurret> CachedTurret;
};
