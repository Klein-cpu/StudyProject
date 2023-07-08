// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Controllers/BaseAIController.h"
#include "BaseAICharacterController.generated.h"

class AAICharacter;
UCLASS()
class MYPROJECT_API ABaseAICharacterController : public ABaseAIController
{
	GENERATED_BODY()

public:
	virtual void SetPawn(APawn* InPawn) override;

	virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float TargetReachRadius = 100.f;
	
private:
	void TryMoveToNextTarget();

	bool IsTargetReached(FVector TargetLocation) const;
	
	TWeakObjectPtr<AAICharacter> CachedAICharacter;

	bool bIsPatrolling = false;	
	
	void SetupPatrolling();
	
};
