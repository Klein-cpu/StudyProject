// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GCBasePawnMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API UGCBasePawnMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

public:
	void JumpStart();

	virtual bool IsFalling() const override;
	
protected:

	UPROPERTY(EditAnywhere)
	float MaxSpeed = 450.0f;

	UPROPERTY(EditAnywhere)
	float InitialJumpVelocity = 500.0f;
	
	UPROPERTY(EditAnywhere)
	bool bEnableGravity = true;

private:
	FVector VerticalVelocity = FVector::ZeroVector;
	bool bIsFalling = false;

};
