// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../InteractiveActor.h"
#include "Zipline.generated.h"

class UCapsuleComponent;
/**
 * 
 */
UCLASS(Blueprintable)
class MYPROJECT_API AZipline : public AInteractiveActor
{
	GENERATED_BODY()

public:
	AZipline();

	virtual void OnConstruction(const FTransform& Transform) override;

	FVector GetFirstPillarTop() const;
	FVector GetSecondPillarTop() const;
	FVector GetWireForwardVector() const;
	float GetWireLength() const;
	bool IsFirstPillarHigher() const;
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float FirstPillarHeight = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float SecondPillarHeight = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float WireFirstFromTopOffset = 3.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float WireSecondFromTopOffset = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float CapsuleComponentRadius = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* FirstPillarMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* SecondPillarMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* WireMeshComponent;
	
	UCapsuleComponent* GetZiplineInteractionCapsule() const;
};
