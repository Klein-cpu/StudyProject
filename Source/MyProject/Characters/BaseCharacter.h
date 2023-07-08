// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCodeTypes.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "MyProject/Components/MovementComponents/BaseCharacterMovementComponent.h"
#include "BaseCharacter.generated.h"


class UCharacterInventoryComponent;
class UInventoryItem;
class UWidgetComponent;
class IInteractable;
class UCharacterAttributesComponent;
class UCharacterEquipmentComponent;
USTRUCT(BlueprintType)
struct FMantlingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* MantlingMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* FPMantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UCurveVector* MantlingCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0, UIMin = 0))
	float MaxHeight = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0, UIMin = 0))
	float MinHeight = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0, UIMin = 0))
	float AnimationCorrectionXY = 65.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0, UIMin = 0))
	float AnimationCorrectionZ = 200.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0, UIMin = 0))
	float MaxHeightStartTime = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0, UIMin = 0))
	float MinHeightStartTime = 0.5f;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingStateChanged, bool)
DECLARE_DELEGATE_OneParam(FOnInteractableObjectFound, FName);

class AInteractiveActor;
class UBaseCharacterMovementComponent;

typedef TArray<AInteractiveActor*, TInlineAllocator<10>> TInteractiveActorsArray;
UCLASS(Abstract, NotBlueprintable)
class MYPROJECT_API ABaseCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	//Basic Methods
	ABaseCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	
	//Basic Components
	FORCEINLINE UBaseCharacterMovementComponent* GetBaseCharacterMovementComponent() const { return BaseCharacterMovement; }
	
	//Basic Movement
	virtual void MoveForward(float Value) {ForwardAxis = Value;}
	virtual void MoveRight(float Value) {RightAxis = Value;}
	virtual void Turn(float Value) {}
	virtual void LookUp(float Value) {}
	
	//Ladders
	virtual void ClimbLadderUp(float Value);
	virtual void InteractWithLadder();
	const ALadder* GetAvailableLadder() const;

	//Ziplines
	virtual void AttachToZipline();
	const AZipline* GetAvailableZipline() const;
	
	//Crouch
	virtual void ChangeCrouchState();

	//Prone
	virtual void RecalculateBaseEyeHeight() override;
	bool bIsProned = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	float PronedEyeHeight;
	virtual void ChangeProneState();
	UFUNCTION(BlueprintCallable, Category=Character)
	virtual bool CanProne() const;
	UFUNCTION(BlueprintCallable, Category=Character, meta=(HidePin="bClientSimulation"))
	virtual void Prone();
	UFUNCTION(BlueprintCallable, Category=Character, meta=(HidePin="bClientSimulation"))
	virtual void UnProne(bool bCrouchWanted);
	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	virtual void OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	virtual void StandUp();

	//Jump
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void Jump() override;
	
	//Mantle
	UFUNCTION(BlueprintCallable)
	virtual void Mantle(bool bForce = false);
	virtual bool CanMantle() const;
	virtual void OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime);
	UPROPERTY(ReplicatedUsing=OnRep_IsMantling)
	bool bIsMantling;
	UFUNCTION()
	void OnRep_IsMantling(bool bWasMantling);
	
	//Sprint
	virtual void StartSprint();
	virtual void StopSprint();
	
	//Swim
	virtual void SwimForward(float Value){}
	virtual void SwimRight(float Value){}
	virtual void SwimUp(float Value){}
	bool IsSwimmingUnderWater() const;
	
	//Inverse Kinematics
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKRightFootOffset(){ return IKRightFootOffset; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKLeftFootOffset(){ return IKLeftFootOffset; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKPelvisOffset(){ return IKPelvisOffset; ;}

	//Interactive Actors
	void RegisterInteractiveActor(AInteractiveActor* InteractiveActor);
	void UnregisterInteractiveActor(AInteractiveActor* InteractiveActor);

	//Slide
	virtual void Slide(bool bForce = false);
	virtual void StopSlide();
	virtual bool CanSlide();
	virtual void OnStartSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	virtual void OnEndSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Slide")
	UAnimMontage* SlideMontage;
	UPROPERTY(ReplicatedUsing=OnRep_IsSliding)
	bool bIsSliding;
	UFUNCTION()
	void OnRep_IsSliding(bool bWasSliding);

	//Fall
	virtual void Falling() override;
	virtual void NotifyJumpApex() override;
	virtual void Landed(const FHitResult& Hit) override;

	//Stamina
	virtual void SetIsOutOfStamina(bool bIsOutOfStamina_In);

	//Equipment
	const UCharacterEquipmentComponent* GetCharacterEquipmentComponent() const;
	UCharacterEquipmentComponent* GetCharacterEquipmentComponent_Mutable();
	
	//Attributes
	UCharacterAttributesComponent* GetCharacterAttributesComponent() const;
	
	//RangeWeapon
	void StartFire();
	void StopFire();
	void Reload() const;
	void ChangeWeaponRegime();

	//Aiming
	void StartAiming();
	void StopAiming();
	bool IsAiming() const;
	float GetAimingMovementSpeed() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void OnStartAiming();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void OnStopAiming();
	FOnAimingStateChanged OnAimingStateChangedEvent;
	FRotator GetAimOffset() const;
	
	//Equipment
	void NextItem();
	void PreviousItem();

	//Items
	void EquipPrimaryItem();

	//MeleeWeapon
	void PrimaryMeleeAttack();
	void SecondaryMeleeAttack();
	
	void Interact();

	FOnInteractableObjectFound OnInteractableObjectFound;

	UPROPERTY(VisibleDefaultsOnly, Category = "Character | Components")
	UWidgetComponent* HealthBarProgressComponent;

	void InitializeHealthProgress();

	bool PickUpItem(TWeakObjectPtr<UInventoryItem> ItemToPickUp);

	void UseInventory(APlayerController* PlayerController);
	
/**IGenericTeamAgentInterface*/
	FGenericTeamId GetGenericTeamId() const override;
/**~IGenericTeamAgentInterface*/
	
protected:
	//Basic Methods
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	//Basic Components	
	UPROPERTY()
	TObjectPtr<UBaseCharacterMovementComponent> BaseCharacterMovement;
	
	//Character Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	UCharacterAttributesComponent* CharacterAttributesComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	UCharacterEquipmentComponent* CharacterEquipmentComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	UCharacterInventoryComponent* CharacterInventoryComponent;
	
	//Sprint
	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement")
	void OnSprintStart();
	virtual void OnSprintStart_Implementation()	{}
	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement")
	void OnSprintEnd();
	virtual void OnSprintEnd_Implementation() {}
	UFUNCTION()
	virtual bool CanSprint();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Sprint")
	float SprintSpeed = 800.f;
	
	//Inverse Kinematics
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Character | IK settings")
	FName RightFootSocketName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character | IK settings")
	FName LeftFootSocketName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character | IK settings", meta = (ClampMin = 0.f, UIMin = 0.f))
	float IKTraceExtendDistance = 50.f;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Character | IK settings", meta = (ClampMin = 0.f, UIMin = 0.f))
	float IKInterpSpeed = 30.f;
	
	//Mantling
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Movement")
	class ULedgeDetectorComponent* LedgeDetectorComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	FMantlingSettings HighMantleSettings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	FMantlingSettings LowMantleSettings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling", meta = (ClampMin = 0.f, UIMin = 0.f))
	float LowMantleMaxHeight = 125.f;

	//WallRun
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun", meta = (UIMin = 0.f, ClampMin = 0.f))
	float MaxWallRunTime = 1.f;

	//Death
	virtual void OnDeath();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Animations")
	UAnimMontage* OnDeathAnimMontage;

	//HardLand
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Animations")
	UAnimMontage* HardLandAnimMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Animations")
	UAnimMontage* FPHardLandAnimMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Falling")
	float MinHardLandHeight = 1000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Attributes")
	UCurveFloat* FallDamageCurve;
	FVector CurrentFallApex;

	//Weapon
	virtual void OnStartAimingInternal();
	virtual void OnStopAimingInternal();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Team")
	ETeams Team = ETeams::Enemy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Team")
	float LineOfSightDistance = 500.f;

	void TraceLineOfSight();

	UPROPERTY()
	TScriptInterface<IInteractable> LineOfSightObject;
	
private:
	//Sprint
	void TryChangeSprintState();
	bool bIsSprintRequested = false;

	//Inverse Kinematics
	float IKRightFootOffset = 0.f;
	float IKLeftFootOffset = 0.f;
	float IKPelvisOffset = 0.f;
	void UpdateIKSettings(float DeltaTime);
	float GetIKOffsetForASocket(const FName& SocketName);
	float CalculateIKPelvisOffset();

	//Prone
	TObjectPtr<USkeletalMeshComponent> BaseMesh;

	//Mantling
	const FMantlingSettings& GetMantlingSettings(float LedgeHeight) const;

	//Interactive Actors
	TInteractiveActorsArray AvailableInteractiveActors;

	//Wall Run
	UFUNCTION()
	void OnPlayerCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	bool IsSurfaceWallRunable(const FVector& SurfaceNormal) const;
	bool AreRequiredKeysDown() const;
	float ForwardAxis = 0.f;
	float RightAxis = 0.f;
	FVector CurrentWallRunDirection = FVector::ZeroVector;

	//Slide
	
	//Death
	FTimerHandle DeathMontageTimer;
	void EnableRagdoll();

	//HardLand
	FTimerHandle HardLandMontageTimer;
	void EnableMovement();

	//Weapon
	bool bIsAiming = false;
	float CurrentAimingMovementSpeed;


};

