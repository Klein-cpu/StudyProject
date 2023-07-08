// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlatformInvocator.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvocatorActivated);

UCLASS()
class MYPROJECT_API APlatformInvocator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlatformInvocator();

	UPROPERTY(BlueprintAssignable)
		FOnInvocatorActivated OnInvocatorActivated;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
		void Invoke();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
