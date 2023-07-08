#include "TraceUtils.h"

bool TraceUtils::SweepLineSingleByChannel(const UWorld* World, FHitResult& OutHit, const FVector& Start, const FVector& End,
										  ECollisionChannel TraceChannel, const FCollisionQueryParams& Params,
										  const FCollisionResponseParams& ResponseParam, bool bDrawDebug, float DrawTime,
										  FColor TraceColor, FColor HitColor)
{
	bool bResult = false;

	bResult = World->LineTraceSingleByChannel(OutHit, Start, End, TraceChannel, Params, ResponseParam);

#if ENABLE_DRAW_DEBUG
	if(bDrawDebug)
	{
		DrawDebugLine(World, Start, End, TraceColor, false, DrawTime);
		if(bResult)
		{
			DrawDebugPoint(World, OutHit.ImpactPoint, 10.f, HitColor, false, DrawTime);
		}
	}
#endif

	return bResult;
}

bool TraceUtils::SweepCapsuleSingleByChannel(const UWorld* World, FHitResult& OutHit, const FVector& Start, const FVector& End,
                                             float CapsuleRadius, float CapsuleHalfHeight, const FQuat& Rot, ECollisionChannel TraceChannel, const FCollisionQueryParams& Params,
                                             const FCollisionResponseParams& ResponseParam, bool bDrawDebug, float DrawTime, FColor TraceColor, FColor HitColor)
{
	bool bResult = false;

	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	bResult = World->SweepSingleByChannel(OutHit, Start, End, Rot, TraceChannel, CollisionShape, Params, ResponseParam);

#if ENABLE_DRAW_DEBUG
	if(bDrawDebug)
	{
		DrawDebugCapsule(World, Start, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, TraceColor, false, DrawTime);
		DrawDebugCapsule(World, End, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, TraceColor, false, DrawTime);
		DrawDebugLine(World, Start, End, FColor::Black, false, DrawTime);
		if(bResult)
		{
			DrawDebugCapsule(World, OutHit.Location, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, HitColor, false, DrawTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 10.f, HitColor, false, DrawTime);
		}
	}
#endif
	
	return bResult;
}

bool TraceUtils::SweepSphereSingleByChannel(const UWorld* World, FHitResult& OutHit, const FVector& Start,
	const FVector& End, float SphereRadius, ECollisionChannel TraceChannel, const FCollisionQueryParams& Params,
	const FCollisionResponseParams& ResponseParam, bool bDrawDebug, float DrawTime, FColor TraceColor, FColor HitColor)
{

	bool bResult = false;

	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(SphereRadius);
	bResult = World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, TraceChannel, CollisionShape, Params, ResponseParam);

#if ENABLE_DRAW_DEBUG
	if(bDrawDebug)
	{
		FVector DebugCenter = (Start + End) * .5f;
		FVector TraceVector = End - Start;
		float DebugCapsuleHalfHeight = TraceVector.Size() * .5f;

		FQuat DebugCapsuleRotation = FRotationMatrix::MakeFromZ(TraceVector).ToQuat();
		
		DrawDebugCapsule(World, DebugCenter, DebugCapsuleHalfHeight, SphereRadius, DebugCapsuleRotation, TraceColor, false, DrawTime);

		if(bResult)
		{
			DrawDebugSphere(World, OutHit.Location, SphereRadius, 16, HitColor, false, DrawTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 10.f, HitColor, false, DrawTime);
		}
	}
#endif
	
	return bResult;
}

bool TraceUtils::OverlapCapsuleAnyByProphile(const UWorld* World, const FVector& Pos, float CapsuleRadius,
	float CapsuleHalfHeight, FQuat Rotation, FName ProfileName, const FCollisionQueryParams& QueryParams,
	bool bDrawDebug, float DrawTime, FColor HitColor)
{
	bool bResult = false;

	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	bResult = World->OverlapAnyTestByProfile(Pos, Rotation, ProfileName, CollisionShape, QueryParams);

#if ENABLE_DRAW_DEBUG
	if(bDrawDebug)
	{
		DrawDebugCapsule(World, Pos, CapsuleHalfHeight, CapsuleRadius, Rotation, HitColor, false, DrawTime);
	}
#endif
	
	return bResult;
}

bool TraceUtils::OverlapCapsuleBlockingByProphile(const UWorld* World, const FVector& Pos, float CapsuleRadius,
	float CapsuleHalfHeight, FQuat Rotation, FName ProfileName, const FCollisionQueryParams& QueryParams,
	bool bDrawDebug, float DrawTime, FColor HitColor)
{
	bool bResult = false;

	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	bResult = World->OverlapBlockingTestByProfile(Pos, Rotation, ProfileName, CollisionShape, QueryParams);

#if ENABLE_DRAW_DEBUG
	if(bDrawDebug)
	{
		DrawDebugCapsule(World, Pos, CapsuleHalfHeight, CapsuleRadius, Rotation, HitColor, false, DrawTime);
	}
#endif
	
	return bResult;
}
