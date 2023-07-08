// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "../Components/MovementComponents/GCBasePawnMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameCodeBasePawn.generated.h"

UCLASS()
class MYPROJECT_API AGameCodeBasePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGameCodeBasePawn();

protected:

	UPROPERTY(VisibleAnywhere)
		UPawnMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere)
		USphereComponent* CollisionComponent;


public:	

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Jump();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetInputForward() {return InputForward;}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetInputRight() {return InputRight;}

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Base pawn")
	float CollisionSphereRadius = 50.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Base pawn")
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Base pawn")
	class UCameraComponent* CameraComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Base pawn")
	class UArrowComponent* ArrowComponent;
#endif

public:
	UFUNCTION()
	void OnBlendComplete();

	AActor* CurrentViewActor;
    
	float InputForward = 0.0f;
	float InputRight = 0.0f;

};
