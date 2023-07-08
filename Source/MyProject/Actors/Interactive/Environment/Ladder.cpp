// Fill out your copyright notice in the Description page of Project Settings.


#include "Ladder.h"

#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "MyProject/GameCodeTypes.h"

ALadder::ALadder()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("LadderRoot"));

	LeftRailMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftRail"));
	LeftRailMeshComponent->SetupAttachment(RootComponent);
	
	RightRailMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightRail"));
	RightRailMeshComponent->SetupAttachment(RootComponent);

	StepsMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Steps"));
	StepsMeshComponent->SetupAttachment(RootComponent);

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(RootComponent);
	InteractionVolume->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	InteractionVolume->SetGenerateOverlapEvents(true);

	TopInteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TopInteractionVolume"));
	TopInteractionVolume->SetupAttachment(RootComponent);
	TopInteractionVolume->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	TopInteractionVolume->SetGenerateOverlapEvents(true);
}

void ALadder::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	LeftRailMeshComponent->SetRelativeLocation(FVector(0.f, -LadderWidth * .5f, LadderHeight * .5f));
	RightRailMeshComponent->SetRelativeLocation(FVector(0.f, LadderWidth * .5f, LadderHeight * .5f));

	UStaticMesh* LeftRailMesh = LeftRailMeshComponent->GetStaticMesh();
	if(IsValid(LeftRailMesh))
	{
		float MeshHeight = LeftRailMesh->GetBoundingBox().GetSize().Z;

		if(!FMath::IsNearlyZero(MeshHeight))
		{
			LeftRailMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, LadderHeight / MeshHeight));
		}
	}
	
	UStaticMesh* RightRailMesh = RightRailMeshComponent->GetStaticMesh();
	if(IsValid(RightRailMesh))
	{
		float MeshHeight = RightRailMesh->GetBoundingBox().GetSize().Z;

		if(!FMath::IsNearlyZero(MeshHeight))
		{
			RightRailMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, LadderHeight / MeshHeight));
		}
	}

	UStaticMesh* StepsMesh = StepsMeshComponent->GetStaticMesh();
	if(IsValid(StepsMesh))
	{
		float MeshWidth = StepsMesh->GetBoundingBox().GetSize().Y;
		if(!FMath::IsNearlyZero(MeshWidth))
		{
			StepsMeshComponent->SetRelativeScale3D(FVector(1.f, LadderWidth / MeshWidth, 1.f));
		}
	}

	StepsMeshComponent->ClearInstances();

	uint32 StepsCount = FMath::FloorToInt((LadderHeight - BottomStepOffset) / StepsInterval);
	for(uint32 i = 0; i < StepsCount; ++i)
	{
		FTransform InstanceTransform(FVector(1.f, 1.f, BottomStepOffset + i * StepsInterval));
		StepsMeshComponent->AddInstance(InstanceTransform);
	}

	float BoxDepthExtent = GetLadderInteractionBox()->GetUnscaledBoxExtent().X;
	GetLadderInteractionBox()->SetBoxExtent(FVector(BoxDepthExtent, LadderWidth * .5f, LadderHeight * .5f));
	GetLadderInteractionBox()->SetRelativeLocation(FVector(BoxDepthExtent, 0.f, LadderHeight * .5f));

	FVector TopBoxExtent = TopInteractionVolume->GetUnscaledBoxExtent();
	TopInteractionVolume->SetBoxExtent(FVector(TopBoxExtent.X, LadderWidth * .5f, TopBoxExtent.Z));
	TopInteractionVolume->SetRelativeLocation(FVector(-TopBoxExtent.X, 0.f, LadderHeight + TopBoxExtent.Z));
}

void ALadder::BeginPlay()
{
	Super::BeginPlay();
	TopInteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &ALadder::OnInteractionVolumeOverlapBegin);
	TopInteractionVolume->OnComponentEndOverlap.AddDynamic(this, &ALadder::OnInteractionVolumeOverlapEnd);
}

float ALadder::GetLadderHeight() const
{
	return LadderHeight;
}

bool ALadder::GetIsOnTop() const
{
	return bIsOnTop;
}

UAnimMontage* ALadder::GetAttachFromTopAnimMontage() const
{
	return AttachFromTopAnimMontage;
}

FVector ALadder::GetAttachFromTopAnimMontageStartingLocation() const
{
	FRotator OrientationRotation = GetActorForwardVector().ToOrientationRotator();
	FVector Offset = OrientationRotation.RotateVector(AttachFromTopAnimMontageInitialOffset);
	
	FVector LadderTop = GetActorLocation() + GetActorUpVector() * LadderHeight;

	return LadderTop + Offset;
}

UBoxComponent* ALadder::GetLadderInteractionBox() const
{
	return StaticCast<UBoxComponent*>(InteractionVolume); 
}

void ALadder::OnInteractionVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnInteractionVolumeOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if(!IsOverlappingCharacterCapsule(OtherActor, OtherComp))
	{
		return;
	}
	
	if(OverlappedComponent == TopInteractionVolume)
	{
		bIsOnTop = true;
	}
}

void ALadder::OnInteractionVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnInteractionVolumeOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	
	if(!IsOverlappingCharacterCapsule(OtherActor, OtherComp))
	{
		return;
	}
	
	if(OverlappedComponent == TopInteractionVolume)
	{
		bIsOnTop = false;
	}
}
