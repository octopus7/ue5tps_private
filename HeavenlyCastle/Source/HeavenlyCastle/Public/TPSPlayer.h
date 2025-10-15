#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Cover/CoverTypes.h"
#include "TPSPlayer.generated.h"

class UHealthComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UAnimMontage;
class UUserWidget;
class AThrowableGrenade;
class UCoverControllerComponent;
class UCoverCameraComponent;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Unarmed         UMETA(DisplayName = "Unarmed"),
    Armed           UMETA(DisplayName = "Armed"),
    ThrownReady     UMETA(DisplayName = "ThrownReady"),
    Equipping       UMETA(DisplayName = "Equipping"),
    Unequipping     UMETA(DisplayName = "Unequipping")
};

UCLASS()
class HEAVENLYCASTLE_API ATPSPlayer : public ACharacter
{
    GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATPSPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
    // Called every frame
    virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void Jump() override;

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Cover controller component driving new cover system */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	UCoverControllerComponent* CoverController;

	/** Optional camera helper for cover offsets */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
	UCoverCameraComponent* CoverCamera;

	/************************************************************************
	* Camera Control
	************************************************************************/

	/** Default distance of the camera from the player */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float DefaultCameraArmLength;

	/** Distance of the camera from the player when aiming */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float AimingCameraArmLength;

	/** Default socket offset for the camera boom */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector DefaultCameraSocketOffset;

	/** Socket offset for the camera boom when aiming */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector AimingCameraSocketOffset;

	/** Distance of the camera from the player when sprinting */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float SprintCameraArmLength;

	/** Socket offset for the camera boom when sprinting */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector SprintCameraSocketOffset;

	/** Speed at which the camera interpolates to its new position */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraInterpSpeed;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Aim Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

    /** Fire Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* FireAction;

    /** Toggle Thrown-Ready Input Action (optional) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* ThrowReadyAction;

    /** Direct Throw Input Action (optional). If not set, FireAction is reused while ThrownReady. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* ThrowAction;

    /** Sprint Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* SprintAction;

    /** Cover toggle input action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* CoverAction;

    /** Arm/Unarm Toggle Input Action (IA_Arm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* ArmAction;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for aiming input (start) */
	void AimStarted();

	/** Called for aiming input (stop) */
	void AimStopped();

    /** Called for firing input (start) */
    void StartFire();

    /** Called for firing input (stop) */
    void StopFire();

    /** Called to toggle thrown-ready state */
    void ToggleThrownReady();

    /** Called for throw input */
    void StartThrow();

    /** Called when cover input is triggered */
    void HandleCoverAction();


	/** Called for sprint input (start) */
	void SprintStarted();

	/** Called for sprint input (stop) */
	void SprintStopped();

	/** Fires a projectile */
	void Fire();

	/** Computes aim direction from the camera crosshair perspective */
	FVector CalculateCameraAimDirection(const FVector& MuzzleLocation, FVector& OutAimPoint);

	/** Projectile spawn point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* ProjectileSpawnPoint;

    /** Projectile class to spawn */
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    TSubclassOf<class AActor> ProjectileClass; // Using AActor for now, can be changed to a specific projectile class later

    /** Niagara FX to spawn on fire */
    UPROPERTY(EditAnywhere, Category = "Effects")
    class UNiagaraSystem* FireEffect;

    /** Speed used for the projectile trajectory prediction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
    float ProjectilePredictionSpeed;

    /** Maximum distance for the camera-based aim trace */
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float CameraAimTraceDistance;

    /************************************************************************
    * Throwable (Grenade)
    ************************************************************************/

    /** Grenade class to spawn when throwing */
    UPROPERTY(EditDefaultsOnly, Category = "Throwable")
    TSubclassOf<AThrowableGrenade> GrenadeClass;

    /** Initial throw speed for prediction and launch */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable")
    float ThrowSpeed = 1200.f;

    /** Fuse time to set on grenade after throwing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable")
    float GrenadeFuseTime = 2.5f;

    /** Whether to draw predicted throw arc while ThrownReady */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable")
    bool bDrawThrowPrediction = true;

    /** Socket used to hold grenade in ThrownReady (e.g., hand). Must exist on character mesh */
    UPROPERTY(EditDefaultsOnly, Category = "Throwable")
    FName ThrowAttachSocketName = FName("ThrowSocket");

    /** Grenade instance being held while in ThrownReady */
    UPROPERTY(Transient)
    AThrowableGrenade* HeldGrenade;

	/** Default walk speed of the character */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float DefaultWalkSpeed;

	/** Sprint walk speed of the character */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintWalkSpeed;

	/** Time between shots for automatic fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float TimeBetweenShots;

	/************************************************************************
	* Weapon Handling
	************************************************************************/

	/** Blueprint of the weapon to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AActor> WeaponBlueprint;

    /** Socket name on the mesh to attach the weapon to */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName WeaponSocketName;

    /** Socket name for storing weapon on back when unarmed */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName UnarmedBackSocketName;

	/** A reference to the spawned weapon */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon")
	AActor* SpawnedWeapon;

    /** Maximum rounds per magazine */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
    int32 MaxAmmo = 30;

    /** Initial rounds loaded when the game starts */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
    int32 StartingAmmo = 30;

    /** When true, ammo is not consumed while firing (debug/testing) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo")
    bool bInfiniteAmmo = false;

    /** Current rounds available for firing */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ammo")
    int32 CurrentAmmo = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

private:
    /** Timer handle for automatic firing */
    FTimerHandle TimerHandle_AutomaticFire;

	/** Flag to track if the dedicated aim button is pressed */
	bool bIsAiming;

    /** Flag to track if the player is currently firing */
    bool bIsFiring;

    /** Flag to track if the player is sprinting */
    bool bIsSprinting;

    /** Helper to compute throw start location and velocity */
    void ComputeThrowParams(FVector& OutStart, FVector& OutVelocity) const;

    /** Spawn and attach a grenade to ThrowAttachSocket while in ThrownReady */
    void OnEnterThrownReady();

    /** Cleanup held grenade when leaving ThrownReady */
    void OnExitThrownReady();

    /** Updates the character's rotation settings based on aiming and firing states */
    void UpdateRotationSettings();

    UFUNCTION()
    void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    void PushAmmoToUI();
    void PushCoverAvailabilityToUI(bool bAvailable);
    void EvaluateCoverAvailability(float DeltaSeconds);
    void HandleOutOfAmmo();
    bool ConsumeAmmo();

private:
    bool bIsDead;

    UFUNCTION()
    void OnDeath();

public:
    /************************************************************************
    * Combat State (Armed/Unarmed/ThrownReady)
    ************************************************************************/

protected:
    /** Current combat state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    ECombatState CombatState;

    /** Optional montages for equip/unequip transitions */
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation")
    UAnimMontage* EquipMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation")
    UAnimMontage* UnequipMontage;

    /** When to attach/detach relative to montage start if no anim notify */
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation")
    float EquipAttachDelay;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation")
    float UnequipAttachDelay;

    /** Pending target state after unequip (e.g., ThrownReady) */
    bool bHasPendingStateAfterUnequip;
    ECombatState PendingStateAfterUnequip;

    /** Timers for equip/unequip attach moments */
    FTimerHandle TimerHandle_EquipAttach;
    FTimerHandle TimerHandle_UnequipAttach;

    /** Attach helper */
    void AttachWeaponToSocket(const FName& SocketName);

    /** Transition handlers */
    void HandleEquipAttach();
    void HandleUnequipAttach();

    /** Drop the currently held weapon and enable physics on it */
    void DropWeaponWithPhysics();

    /** Impulse strength applied to dropped weapon */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Drop")
    float WeaponDropForwardImpulse = 150.f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Drop")
    float WeaponDropUpImpulse = 120.f;

    /** UI: simple widget hook */
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> CombatStateWidgetClass;

    UPROPERTY()
    UUserWidget* CombatStateWidgetInstance;

    void UpdateCombatStateUI();

    /** Ensure attached weapon doesn't block the player or world while equipped/holstered */
    void ConfigureWeaponCollision();

public:
    /** Requests a state switch with animations */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RequestSetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ToggleArmed();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetThrownReady();

    /** Perform the actual throw (spawn and launch grenade) */
    void ThrowGrenade();

    /************************************************************************
    * Debug
    ************************************************************************/
public:
    /** Toggle movement debug overlay (Exec console: ToggleMovementDebug) */
    UFUNCTION(Exec)
    void ToggleMovementDebug();

protected:
    /** When true, prints speed, max speed, sprint, input magnitude, camera length each tick */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
    bool bMovementDebugEnabled = false;

    /************************************************************************
    * Cover System
    ************************************************************************/

protected:
    /** Threshold of backward input magnitude to exit cover intentionally */
    UPROPERTY(EditDefaultsOnly, Category = "Cover")
    float CoverExitForwardThreshold = 0.6f;

private:
    /** Cached offset provided by cover camera component */
    FVector CoverCameraOffset = FVector::ZeroVector;

    /** Cached result of the most recent cover availability query */
    bool bCachedCoverAvailable = false;

    /** Time accumulator used to throttle cover availability polling */
    float CoverAvailabilityElapsed = 0.f;

    /** Polling interval (seconds) for cover availability updates */
    UPROPERTY(EditDefaultsOnly, Category = "Cover|UI", meta = (ClampMin = "0.0"))
    float CoverAvailabilityPollInterval = 0.15f;

    bool IsInCover() const;

    UFUNCTION()
    void OnCoverStateChanged(ESimpleCoverState NewState);

    UFUNCTION()
    void OnCoverCameraOffsetUpdated(FVector Offset);
};
