// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BaseCharacterMovementComponent.generated.h"

USTRUCT()
struct FMantlingMovementParameters
{
	GENERATED_BODY()
	
	FVector InitialLocation = FVector::ZeroVector;
	FRotator InitialRotation = FRotator::ZeroRotator;
	
	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;

	FVector InitialAnimationLocation = FVector::ZeroVector;
	
	UPROPERTY()
	UMeshComponent* TargetMesh;
	FVector InitialTargetMeshLocation = FVector::ZeroVector;
	
	float Duration = 1.f;
	float StartTime = 0.f;

	UPROPERTY()
	UCurveVector* MantlingCurve;
};

UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	CMOVE_None = 0	UMETA(Displayname = "None"),
	CMOVE_Mantling	UMETA(Displayname = "Mantling"),
	CMOVE_Ladder	UMETA(Displayname = "Ladder"),
	CMOVE_Zipline   UMETA(Displayname = "Zipline"),
	CMOVE_WallRun   UMETA(Displayname = "WallRun"),
	CMOVE_Slide		UMETA(Displayname = "Slide"),
	CMOVE_Max		UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDetachFromLadderMethod : uint8
{
	Fall = 0,
	ReachingTheTop,
	ReachingTheBottom,
	JumpOff
};

UENUM()
enum class EWallRunSide : uint8
{
	None,
	Left,
	Right
};

class ABaseCharacter;
/**
 * 
 */
UCLASS()
class MYPROJECT_API UBaseCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	friend class FSavedMove_GC;
	
public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void BeginPlay() override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void PhysicsRotation(float DeltaTime) override;

	ABaseCharacter* GetBaseCharacterOwner() const; 
	
	FORCEINLINE bool IsSprinting() { return bIsSprinting; }
	FORCEINLINE bool IsOutOfStamina() { return bIsOutOfStamina; }
	FORCEINLINE bool IsProning() const;
	
	void SetIsOutOfStamina(bool bIsOutOfStamina_In)
	{
		bIsOutOfStamina = bIsOutOfStamina_In;
		if(bIsOutOfStamina)
		{
			StopSprint();
		}
	}
	virtual float GetMaxSpeed() const override;

	void StartSprint();
	void StopSprint();
	
	void StartMantle(const FMantlingMovementParameters& MantlingParameters);
	void EndMantle();
	void PhysMantling(float DeltaTime, int32 Iterations);
	bool IsMantling() const;
	
	void Prone();
	void UnProne(bool bWantsToProne);
	bool bWantsToProne = false;
	bool bWantsCrouchFromProne = false;

	void AttachToLadder(const class ALadder* Ladder);
	void DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod = EDetachFromLadderMethod::Fall);
	void PhysLadder(float DeltaTime, int32 Iterations);
	bool IsOnLadder() const;
	const ALadder* GetCurrentLadder();
	float GetActorToCurrentLadderProjection(const FVector& Location) const;
	float GetLadderSpeedRatio() const;

	void AttachToZipline(const class AZipline* Zipline);
	void DetachFromZipline();
	void PhysZipline(float DeltaTime, int32 Iterations);
	bool IsOnZipline() const;
	const AZipline* GetCurrentZipline() const;
	float GetActorToCurrentZiplineProjection(const FVector& Location) const;
	
	void PhysWallRun(float DeltaTime, int32 Iterations);
	
	void StartWallRun(EWallRunSide Side, const FVector& Direction);
	bool bCanWallRun = true;
	bool IsWallRunning() const;
	EWallRunSide GetWallRunSide() const;
	bool bWantsToWallJump;
	void JumpFromWall();
	void StopWallRun();
	void GetWallRunSideAndDirection(const FVector& HitNormal, EWallRunSide& OutSide, FVector& OutDirection) const;

	void StartSlide();
	void StopSlide();
	void PhysSliding(float DeltaTime, int32 Iterations);
	bool IsSliding() const;
	
protected:
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	
	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<ABaseCharacter> BaseCharacterOwner;
	
	//Sprint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: sprint", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SprintSpeed = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: sprint")
	float OutOfStaminaSpeed = 100.f;

	//Prone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: prone", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxProneSpeed = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: prone", meta = (ClampMin = 0.f, UIMin = 0.f));
	float PronedCapsuleRadius = 40.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: prone", meta = (ClampMin = 0.f, UIMin = 0.f));
	float PronedHalfHeight = 40.f;

	//Swimming
	UPROPERTY(Category = "Character Movement: Swimming", EditAnywhere, BlueprintReadWrite)
	float SwimmingCapsuleRadius = 60.f;
	
	UPROPERTY(Category = "Character Movement: Swimming", EditAnywhere, BlueprintReadWrite)
	float SwimmingCapsuleHalfHeight = 60.f;

	//Ladders
	UPROPERTY(Category = "Character Movement: Ladder", EditAnywhere, BlueprintReadWrite)
	float ClimbingOnLadderMaxSpeed = 200.f;
	
	UPROPERTY(Category = "Character Movement: Ladder", EditAnywhere, BlueprintReadWrite)
	float ClimbingOnLadderBrakingDeceleration = 2048.f;

	UPROPERTY(Category = "Character Movement: Ladder", EditAnywhere, BlueprintReadWrite)
	float LadderToCharacterOffset = 60.f;

	UPROPERTY(Category = "Character Movement: Ladder", EditAnywhere, BlueprintReadWrite)
	float MaxLadderTopOffset = 90.f;

	UPROPERTY(Category = "Character Movement: Ladder", EditAnywhere, BlueprintReadWrite)
	float MinLadderBottomOffset = 90.f;

	UPROPERTY(Category = "Character Movement: Ladder", EditAnywhere, BlueprintReadWrite)
	float JumpOffFromLadderSpeed = 500.f;

	//Ziplines
	UPROPERTY(Category = "Character Movement: Zipline", EditAnywhere, BlueprintReadWrite)
	float ZiplineSlidingSpeed = 250.f;
	
	UPROPERTY(Category = "Character Movement: Zipline", EditAnywhere, BlueprintReadWrite)
	float CharacterZiplineOffset = 119.f;

	UPROPERTY(Category = "Character Movement: Zipline", EditAnywhere, BlueprintReadWrite)
	float ZiplineEndOffset = 68.f;

	//WallRun
	UPROPERTY(Category = "Character Movement: Wall Run", EditAnywhere, BlueprintReadWrite)
	float MaxWallRunTime = 2.f;

	UPROPERTY(Category = "Character Movement: Wall Run", EditAnywhere, BlueprintReadWrite)
	float MaxWallRunSpeed = 350.f;

	//Slide
	UPROPERTY(Category = "Character Movement: Slide", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SlideSpeed = 1000.f;

	UPROPERTY(Category = "Character Movement: Slide", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SlideCapsuleHalfHeight = 60.f;

	UPROPERTY(Category = "Character Movement: Slide", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SlideMaxTime = 2.f;
	
private:
	bool bIsSprinting;
	bool bIsOutOfStamina;
	bool bIsWallRunning = false;

	FMantlingMovementParameters CurrentMantlingParameters;

	FTimerHandle MantlingTimer;

	UPROPERTY()
	const ALadder* CurrentLadder = nullptr;

	UPROPERTY()
	const AZipline* CurrentZipline = nullptr;

	FRotator ForcedTargetRotation = FRotator::ZeroRotator;
	bool bForceRotation = false;
	
	
	EWallRunSide CurrentWallRunSide = EWallRunSide::None;
	FVector CurrentWallRunDirection = FVector::ZeroVector;

	FTimerHandle WallRunTimer;

	FTimerHandle SlidingTimer;
};


class FSavedMove_GC : public FSavedMove_Character
{
	typedef FSavedMove_Character Super;

public:
	virtual void Clear() override;

	virtual uint8 GetCompressedFlags() const override;

	virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const override;

	virtual void SetMoveFor(ACharacter* InCharacter, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

	virtual void PrepMoveFor(ACharacter* Character) override;
	
private:
	uint8 bSavedIsSprinting : 1;
	uint8 bSavedIsMantling : 1;
	uint8 bSavedIsSliding : 1;
};

class FNetworkPredictionData_Client_Character_GC : public FNetworkPredictionData_Client_Character
{
	typedef FNetworkPredictionData_Client_Character Super;

public:
	FNetworkPredictionData_Client_Character_GC(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};