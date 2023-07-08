// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Weapon/MeleeHitRegistrator.h"

#include "GameCodeTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/DebugSubsystem.h"
#include "Utils/TraceUtils.h"

UMeleeHitRegistrator::UMeleeHitRegistrator()
{
	PrimaryComponentTick.bCanEverTick = true;
	SphereRadius = 5.f;
	UPrimitiveComponent::SetCollisionProfileName(CollisionProfileNoCollision);
}

void UMeleeHitRegistrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(bIsHitRegistrationEnabled)
	{
		ProcessHitRegistration();
	}
	PreviousComponentLocation = GetComponentLocation();
}

void UMeleeHitRegistrator::ProcessHitRegistration()
{
	FVector CurrentLocation = GetComponentLocation();

	FHitResult HitResult;

#if ENABLE_DRAW_DEBUG
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryMeleeWeapon);
#else
	bool bIsDebugEnabled = false;
#endif

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner()->GetOwner());
	
	bool bHasHit = TraceUtils::SweepSphereSingleByChannel(
		GetWorld(),
		HitResult,
		PreviousComponentLocation,
		CurrentLocation,
		GetScaledSphereRadius(),
		ECC_MELEE,
		QueryParams,
		FCollisionResponseParams::DefaultResponseParam,
		bIsDebugEnabled,
		5.f
	);

	if(bHasHit)
	{
		FVector Direction = (CurrentLocation-PreviousComponentLocation).GetSafeNormal();
		if(OnMeleeHitRegistered.IsBound())
		{
			OnMeleeHitRegistered.Broadcast(HitResult, Direction);
		}
	}
}

void UMeleeHitRegistrator::SetIsHitRegistrationEnabled(bool bIsEnabled_In)
{
	bIsHitRegistrationEnabled = bIsEnabled_In;
}

