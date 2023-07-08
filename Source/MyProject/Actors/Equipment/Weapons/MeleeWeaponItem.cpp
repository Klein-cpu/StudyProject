// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeaponItem.h"

#include "Characters/BaseCharacter.h"
#include "Components/Weapon/MeleeHitRegistrator.h"

AMeleeWeaponItem::AMeleeWeaponItem()
{
	EquippedSocketName = SocketCharacterWeapon;
}

void AMeleeWeaponItem::StartAttack(EMeleeAttackTypes AttackType)
{
	ABaseCharacter* CharacterOwner = GetCharacterOwner();
	if(!IsValid(CharacterOwner))
	{
		return;	
	}

	HitActors.Empty();
	CurrentAttack = Attacks.Find(AttackType);
	if(CurrentAttack && IsValid(CurrentAttack->AttackMontage))
	{
		UAnimInstance* CharacterAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
		if(IsValid(CharacterAnimInstance))
		{
			float Duration = CharacterAnimInstance->Montage_Play(CurrentAttack->AttackMontage, 1, EMontagePlayReturnType::Duration);
			GetWorld()->GetTimerManager().SetTimer(AttackTimer, this, &AMeleeWeaponItem::OnAttackTimerElapsed, Duration, false);
		}
	}
	else
	{
		OnAttackTimerElapsed();
	}
}

void AMeleeWeaponItem::SetIsHitRegistrationEnabled(bool bIsRegistrationEnabled_In)
{
	HitActors.Empty();
	for(auto HitRegistrator : HitRegistrators)
	{
		HitRegistrator->SetIsHitRegistrationEnabled(bIsRegistrationEnabled_In);
	}
}

void AMeleeWeaponItem::BeginPlay()
{
	Super::BeginPlay();
	GetComponents<UMeleeHitRegistrator>(HitRegistrators);
	for(const auto HitRegistrator : HitRegistrators)
	{
		HitRegistrator->OnMeleeHitRegistered.AddUFunction(this, FName("ProcessHit"));
	}
}

void AMeleeWeaponItem::ProcessHit(const FHitResult& HitResult, const FVector& HitDirection)
{
	if(CurrentAttack == nullptr)
	{
		return;
	}
	
	AActor* HitActor = HitResult.GetActor();
	if(!IsValid(HitActor))
	{
		return;
	}

	if(HitActors.Contains(HitActor))
	{
		return;
	}

	FPointDamageEvent DamageEvent;
	DamageEvent.HitInfo = HitResult;
	DamageEvent.ShotDirection = HitDirection;
	DamageEvent.DamageTypeClass = CurrentAttack->DamageTypeClass;

	ABaseCharacter* CharacterOwner = GetCharacterOwner();
	AController* Controller = IsValid(CharacterOwner) ? CharacterOwner->GetController() : nullptr;
	HitActor->TakeDamage(CurrentAttack->DamageAmount, DamageEvent,  Controller, GetOwner());

	HitActors.Add(HitActor);
}

void AMeleeWeaponItem::OnAttackTimerElapsed()
{
	CurrentAttack = nullptr;
	SetIsHitRegistrationEnabled(false);
}
