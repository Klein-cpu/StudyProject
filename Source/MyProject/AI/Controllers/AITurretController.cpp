// Fill out your copyright notice in the Description page of Project Settings.


#include "AITurretController.h"

#include "AI/Characters/Turret.h"
#include "Characters/BaseCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"

void AAITurretController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if(IsValid(InPawn))
	{
		checkf(InPawn->IsA<ATurret>(), TEXT("AAITurretController::SetPawn AAITurretController can possess only Turrets"));
		CachedTurret = StaticCast<ATurret*>(InPawn);
		CachedTurret->OnTakeAnyDamage.AddDynamic(this, &AAITurretController::OnTakeDamage);
	}
	else
	{
		CachedTurret = nullptr;
	}
}

void AAITurretController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	Super::ActorsPerceptionUpdated(UpdatedActors);

	if(!CachedTurret.IsValid())
	{
		return;
	}
	
	AActor*
		ClosestActor = GetClosestSensedActor(UAISense_Sight::StaticClass());

	// AActor*
	// 	ClosestDamageCauser = GetClosestSensedActor(UAISense_Damage::StaticClass());

	CachedTurret->CurrentTarget = ClosestActor;
	CachedTurret->OnCurrentTargetSet();
}

void AAITurretController::OnTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	ABaseCharacter* CharacterDamageCauser = static_cast<ABaseCharacter*>(DamageCauser->GetOwner());
	FVector EventLocation = DamageCauser->GetActorLocation();
	
	if(IsValid(CharacterDamageCauser))
	{
		EventLocation = CharacterDamageCauser->GetOwner()->GetActorLocation();	
	}
	
	UAISense_Damage::ReportDamageEvent(GetWorld(), DamagedActor, CharacterDamageCauser, Damage,
									   EventLocation, DamagedActor->GetActorLocation());
}

