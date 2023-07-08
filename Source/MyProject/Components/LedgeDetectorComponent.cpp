// Fill out your copyright notice in the Description page of Project Settings.


#include "LedgeDetectorComponent.h"

#include "../GameCodeTypes.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "MyProject/BaseGameInstance.h"
#include "MyProject/Characters/BaseCharacter.h" 
#include "MyProject/Subsystems/DebugSubsystem.h"
#include "MyProject/Utils/TraceUtils.h"

// Called when the game starts
void ULedgeDetectorComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()-IsA<ABaseCharacter>(), TEXT("ULedgeDetectorComponent::BeginPlay() only a character can use ULedgeDetectorComponent"));
	CachedCharacterOwner = StaticCast<ABaseCharacter*>(GetOwner());
	
}

bool ULedgeDetectorComponent::DetectLedge(OUT FLedgeDescription& LedgeDescription)
{
	UCapsuleComponent* CapsuleComponent = CachedCharacterOwner->GetCapsuleComponent();
	const ABaseCharacter* DefaultCharacter = GetDefault<ABaseCharacter>(CachedCharacterOwner->GetClass());
	const UCapsuleComponent* DefaultCapsule = DefaultCharacter->GetCapsuleComponent();

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(GetOwner());

#if ENABLE_DRAW_DEBUG
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryLedgeDetection);
#else
	bool bIsDebugEnabled = false;
#endif
	
	float DrawTime = 5.f;
	
	float BottomZOffset = 2.f;
	FVector CharacterBottom = CachedCharacterOwner->GetActorLocation() - (CapsuleComponent->GetScaledCapsuleHalfHeight() - BottomZOffset) * FVector::UpVector;

	//Upward check
	FHitResult UpwardCheckHitResult;
	
	FVector UpwardStartLocation = CachedCharacterOwner->GetActorLocation();
	FVector UpwardEndLocation = CachedCharacterOwner->GetActorLocation() + MaximumLedgeHeight * FVector::UpVector;

	if(TraceUtils::SweepLineSingleByChannel(GetWorld(), UpwardCheckHitResult, UpwardStartLocation, UpwardEndLocation, ECC_CLIMBING, QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, DrawTime))
	{
		return false;
	}
	
	//Forward check
	FHitResult ForwardCheckHitResult;
	float ForwardCheckCapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	float ForwardCheckCapsuleHalfHeight = (MaximumLedgeHeight - MinimumLedgeHeight) * .5f;

	FVector ForwardStartLocation = CharacterBottom + (MinimumLedgeHeight + ForwardCheckCapsuleHalfHeight) * FVector::UpVector;
	FVector ForwardEndLocation = ForwardStartLocation + CachedCharacterOwner->GetActorForwardVector() * ForwardCheckDistance;

	if(!TraceUtils::SweepCapsuleSingleByChannel(GetWorld(), ForwardCheckHitResult, ForwardStartLocation, ForwardEndLocation, ForwardCheckCapsuleRadius, ForwardCheckCapsuleHalfHeight, FQuat::Identity, ECC_CLIMBING, QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, DrawTime))
	{
		return false;
	}
	
	//Downward check
	FHitResult DownwardCheckHitResult;
	float DownwardSphereCheckRadius = CapsuleComponent->GetScaledCapsuleRadius();

	float DownwardCheckDepthOffset = 10.f;
	FVector DownwardStartLocation = ForwardCheckHitResult.ImpactPoint - ForwardCheckHitResult.ImpactNormal * DownwardCheckDepthOffset;
	DownwardStartLocation.Z = CharacterBottom.Z + MaximumLedgeHeight + CapsuleComponent->GetScaledCapsuleRadius();
	FVector DownwardEndLocation(DownwardStartLocation.X, DownwardStartLocation.Y, CharacterBottom.Z);

	if(!TraceUtils::SweepSphereSingleByChannel(GetWorld(), DownwardCheckHitResult, DownwardStartLocation, DownwardEndLocation, DownwardSphereCheckRadius, ECC_CLIMBING, QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, DrawTime))
	{
		return false;
	}
	
	//Overlap check
	float OverlapCapsuleHalfHeight = DefaultCapsule->GetScaledCapsuleHalfHeight();
	float OverlapCapsuleRadius = DefaultCapsule->GetScaledCapsuleRadius();
	float OverlapCapsuleFloorOffset = 2.f;
	FVector OverlapLocation = DownwardCheckHitResult.ImpactPoint + (OverlapCapsuleHalfHeight + OverlapCapsuleFloorOffset) * FVector::UpVector;

	if(TraceUtils::OverlapCapsuleBlockingByProphile(GetWorld(), OverlapLocation, OverlapCapsuleRadius, OverlapCapsuleHalfHeight, FQuat::Identity, CollisionProfilePawn, QueryParams, bIsDebugEnabled, DrawTime))
	{
		return false;
	}

	LedgeDescription.Location = OverlapLocation;
	LedgeDescription.Rotation = (ForwardCheckHitResult.ImpactNormal * FVector(-1.f, -1.f, 0.f)).ToOrientationRotator();
	LedgeDescription.LedgeNormal = ForwardCheckHitResult.ImpactNormal;
	LedgeDescription.HittedMesh = Cast<UMeshComponent>(DownwardCheckHitResult.GetComponent());
	
	return true;
}

