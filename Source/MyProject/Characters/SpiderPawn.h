// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCodeBasePawn.h"
#include "SpiderPawn.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API ASpiderPawn : public AGameCodeBasePawn
{
	GENERATED_BODY()

public:
	ASpiderPawn();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKRightFrontFootOffset() const {return IKRightFrontFootOffset;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKRightRearFootOffset() const {return IKRightRearFootOffset;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKLeftFrontFootOffset() const {return IKLeftFrontFootOffset;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKLeftRearFootOffset() const {return IKLeftRearFootOffset;}

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spider bot")
	USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spider bot|IK settings")
	FName RightFrontFootSocketName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spider bot|IK settings")
	FName RightRearFootSocketName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spider bot|IK settings")
	FName LeftFrontFootSocketName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spider bot|IK settings")
	FName LeftRearFootSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spider bot|IK settings", meta = (ClampMin = 0.f, UIMin = 0.f))
	float IKTraceExtendDistance = 30.f;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Spider bot|IK settings", meta = (ClampMin = 0.f, UIMin = 0.f))
	float IKInterpSpeed = 20.f;

private:
	float GetIKOffsetForASocket(const FName& SocketName);
	
	float IKRightFrontFootOffset = 0.f;
	float IKRightRearFootOffset = 0.f;
	float IKLeftFrontFootOffset = 0.f;
	float IKLeftRearFootOffset = 0.f;

	float IKTraceDistance = 0.f;
	float IKScale = 0.f;
};
