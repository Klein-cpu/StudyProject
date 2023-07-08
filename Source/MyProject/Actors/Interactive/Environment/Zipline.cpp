// Fill out your copyright notice in the Description page of Project Settings.


#include "Zipline.h"

#include "Components/CapsuleComponent.h"
#include "MyProject/GameCodeTypes.h"

AZipline::AZipline()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ZiplineRoot"));
	
	FirstPillarMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FristPillar"));
	FirstPillarMeshComponent->SetupAttachment(RootComponent);

	SecondPillarMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SecondPillar"));
	SecondPillarMeshComponent->SetupAttachment(RootComponent);

	WireMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wire"));
	WireMeshComponent->SetupAttachment(RootComponent);

	InteractionVolume = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(RootComponent);
	InteractionVolume->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	InteractionVolume->SetGenerateOverlapEvents(true);
}

void AZipline::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	const UStaticMesh* FirstPillarMesh = FirstPillarMeshComponent->GetStaticMesh();
	if(IsValid(FirstPillarMesh))
	{
		const float MeshHeight = FirstPillarMesh->GetBoundingBox().GetSize().Z;

		if(!FMath::IsNearlyZero(MeshHeight))
		{
			FirstPillarMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, FirstPillarHeight / MeshHeight));
		}
	}

	const UStaticMesh* SecondPillarMesh = SecondPillarMeshComponent->GetStaticMesh();
	if(IsValid(SecondPillarMesh))
	{
		const float MeshHeight = SecondPillarMesh->GetBoundingBox().GetSize().Z;

		if(!FMath::IsNearlyZero(MeshHeight))
		{
			SecondPillarMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, SecondPillarHeight / MeshHeight));
		}
	}

	FVector FirstPillarTop = FirstPillarMeshComponent->GetRelativeLocation() + (FirstPillarHeight * .5f - WireFirstFromTopOffset) * FVector::UpVector;
	FVector SecondPillarTop = SecondPillarMeshComponent->GetRelativeLocation() + (SecondPillarHeight * .5f - WireSecondFromTopOffset) * FVector::UpVector;;

	FVector TargetWireCenter = FMath::Lerp(FirstPillarTop, SecondPillarTop, .5f);
	const float TargetWireLength = GetWireLength();
	const FRotator TargetWireRotation = FRotationMatrix::MakeFromX(FirstPillarTop - SecondPillarTop).Rotator();
	
	WireMeshComponent->SetRelativeLocation(TargetWireCenter);
	
	const UStaticMesh* WireMesh = WireMeshComponent->GetStaticMesh();
	if(IsValid(WireMesh))
	{
		const float MeshLength = WireMesh->GetBoundingBox().GetSize().X;
		
		if(!FMath::IsNearlyZero(MeshLength))
		{
			WireMeshComponent->SetRelativeScale3D(FVector(TargetWireLength * .01f, 1.f, 1.f));
		}
		
		WireMeshComponent->SetRelativeRotation(TargetWireRotation);
		
	}

	GetZiplineInteractionCapsule()->SetCapsuleSize(CapsuleComponentRadius, TargetWireLength * .5f);
	GetZiplineInteractionCapsule()->SetRelativeLocation(TargetWireCenter);
	GetZiplineInteractionCapsule()->SetRelativeRotation(FRotator(TargetWireRotation.Pitch + 90.f, TargetWireRotation.Yaw, TargetWireRotation.Roll));
	
	
}

FVector AZipline::GetFirstPillarTop() const
{
	return FirstPillarMeshComponent->GetComponentLocation() + (FirstPillarHeight * .5f - WireFirstFromTopOffset) * FVector::UpVector;
}

FVector AZipline::GetSecondPillarTop() const
{
	return SecondPillarMeshComponent->GetComponentLocation() + (SecondPillarHeight * .5f - WireSecondFromTopOffset) * FVector::UpVector;;
}

FVector AZipline::GetWireForwardVector() const
{
	return WireMeshComponent->GetForwardVector();
}

float AZipline::GetWireLength() const
{
	FVector FirstPillarTop = FirstPillarMeshComponent->GetRelativeLocation() + (FirstPillarHeight * .5f - WireFirstFromTopOffset) * FVector::UpVector;
	FVector SecondPillarTop = SecondPillarMeshComponent->GetRelativeLocation() + (SecondPillarHeight * .5f - WireSecondFromTopOffset) * FVector::UpVector;;

	return FVector::Distance(FirstPillarTop, SecondPillarTop);
}

bool AZipline::IsFirstPillarHigher() const
{
	return GetFirstPillarTop().Z > GetSecondPillarTop().Z;
}

UCapsuleComponent* AZipline::GetZiplineInteractionCapsule() const
{
	return StaticCast<UCapsuleComponent*>(InteractionVolume);
}
