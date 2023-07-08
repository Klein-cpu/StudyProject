// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AIComponents/AIPatrollingComponent.h"

#include "Actors/Navigation/PatrollingPath.h"

bool UAIPatrollingComponent::CanPatrol() const
{
	return IsValid(PatrollingPath) && PatrollingPath->GetWayPoints().Num() > 0;
}

FVector UAIPatrollingComponent::SelectClosestWayPoint()
{
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	const TArray<FVector> WayPoints = PatrollingPath->GetWayPoints();
	FTransform PathTransform = PatrollingPath->GetActorTransform();

	FVector ClosestWayPoint;
	float MinSqDistance = FLT_MAX;
	for(int32 i = 0; i < WayPoints.Num(); i++)
	{
		FVector WayPointWorld = PathTransform.TransformPosition(WayPoints[i]);
		float CurrentSqDistance = (OwnerLocation - WayPointWorld).SizeSquared();
		if(CurrentSqDistance < MinSqDistance)
		{
			MinSqDistance = CurrentSqDistance;
			ClosestWayPoint = WayPointWorld;
			CurrentWayPointIndex = i;
		}
	}
	return ClosestWayPoint;
}

FVector UAIPatrollingComponent::SelectNextWayPoint()
{
	const TArray<FVector> WayPoints = PatrollingPath->GetWayPoints();
	if(CurrentWayPointIndex == PatrollingPath->GetWayPoints().Num()-1)
	{
		if(PatrollingType == EPatrollingType::Circle)
		{
			CurrentWayPointIndex = -1;
		}
		else
		{
			bReverseMovementIsOn = true;
		}
	}
	if(bReverseMovementIsOn && CurrentWayPointIndex == 0)
	{
		bReverseMovementIsOn = false;
	}
	bReverseMovementIsOn ? --CurrentWayPointIndex : ++CurrentWayPointIndex;
	FTransform PathTransform = PatrollingPath->GetActorTransform();
	FVector WayPoint = PathTransform.TransformPosition(WayPoints[CurrentWayPointIndex]);

	return WayPoint;
}