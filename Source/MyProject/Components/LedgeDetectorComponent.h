// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LedgeDetectorComponent.generated.h"

USTRUCT(BlueprintType)
struct FLedgeDescription
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge description")
	FVector Location;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge description")
	FVector LedgeNormal;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge description")
	FRotator Rotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge description")
	UMeshComponent* HittedMesh;
	
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API ULedgeDetectorComponent : public UActorComponent
{
	GENERATED_BODY()
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	bool DetectLedge(OUT FLedgeDescription& LedgeDescription);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection settings", meta = (UIMin = 0.f, ClampMin = 0.f))
	float MinimumLedgeHeight = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection settings", meta = (UIMax = 0.f, ClampMax = 0.f))
	float MaximumLedgeHeight = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection settings", meta = (UIMax = 0.f, ClampMax = 0.f))
	float ForwardCheckDistance = 100.f;
	
private:
	TWeakObjectPtr<class ABaseCharacter> CachedCharacterOwner;
		
};
