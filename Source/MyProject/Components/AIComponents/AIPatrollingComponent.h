// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIPatrollingComponent.generated.h"

UENUM(BlueprintType)
enum class EPatrollingType : uint8
{
	Circle,
	PingPong
};

class APatrollingPath;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYPROJECT_API UAIPatrollingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	bool CanPatrol() const;
	
	FVector SelectClosestWayPoint();
	FVector SelectNextWayPoint();
	
protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Path")
	APatrollingPath* PatrollingPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	EPatrollingType PatrollingType = EPatrollingType::Circle;
	
private:
	int32 CurrentWayPointIndex = -1;

	bool bReverseMovementIsOn = false;
	
};
