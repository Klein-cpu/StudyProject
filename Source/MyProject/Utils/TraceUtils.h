#pragma once

namespace TraceUtils
{
	bool SweepLineSingleByChannel(const UWorld* World, struct FHitResult& OutHit, const FVector& Start,const FVector& End,
									 ECollisionChannel TraceChannel, const FCollisionQueryParams& Params,
									 const FCollisionResponseParams& ResponseParam, bool bDrawDebug = false, float DrawTime = -1.0f,
									 FColor TraceColor = FColor::Black, FColor HitColor = FColor::Red);
	bool SweepCapsuleSingleByChannel(const UWorld* World, struct FHitResult& OutHit, const FVector& Start,const FVector& End,
									 float CapsuleRadius, float CapsuleHalfHeight, const FQuat& Rot, ECollisionChannel TraceChannel, const FCollisionQueryParams& Params,
									 const FCollisionResponseParams& ResponseParam, bool bDrawDebug = false, float DrawTime = -1.0f,
									 FColor TraceColor = FColor::Black, FColor HitColor = FColor::Red);
	bool SweepSphereSingleByChannel(const UWorld* World, struct FHitResult& OutHit, const FVector& Start,const FVector& End,
									 float SphereRadius,ECollisionChannel TraceChannel, const FCollisionQueryParams& Params,
									 const FCollisionResponseParams& ResponseParam, bool bDrawDebug = false, float DrawTime = -1.0f,
									 FColor TraceColor = FColor::Black, FColor HitColor = FColor::Red);
	bool OverlapCapsuleAnyByProphile(const UWorld* World, const FVector& Pos, float CapsuleRadius, float CapsuleHalfHeight, FQuat Rotation,
									 FName ProfileName, const FCollisionQueryParams& QueryParams,bool bDrawDebug = false, float DrawTime = -1.0f, FColor HitColor = FColor::Red);
	bool OverlapCapsuleBlockingByProphile(const UWorld* World, const FVector& Pos, float CapsuleRadius, float CapsuleHalfHeight, FQuat Rotation,
									 FName ProfileName, const FCollisionQueryParams& QueryParams,bool bDrawDebug = false, float DrawTime = -1.0f, FColor HitColor = FColor::Red);
}
