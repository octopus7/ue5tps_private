#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPSPlayer.generated.h"

class UHealthComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UAnimMontage;
class UUserWidget;
class AThrowableGrenade;

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

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

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

	/** Cover Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CoverAction;

    /** Sprint Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* SprintAction;

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

	/** Called for cover input */
	void Cover();

	/** Called for sprint input (start) */
	void SprintStarted();

	/** Called for sprint input (stop) */
	void SprintStopped();

	/** Fires a projectile */
	void Fire();

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

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UHealthComponent* HealthComponent;

protected:
	/************************************************************************
	* Cover System
	************************************************************************/

	/** Tries to find cover and enter it */
	void TryEnterCover();

	/** Exits from the current cover */
	void ExitCover();

	/** True if player is in cover. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover")
	bool bIsCovered;

	/** The normal of the cover surface */
	FVector CoverWallNormal;

	/************************************************************************
	* Enter Cover Animation
	************************************************************************/
	/** True if player is currently entering cover with animation. */
	bool bIsEnteringCover;

	/** Time when the enter cover animation started. */
	float EnterCoverStartTime;

	/** Duration of the enter cover animation. */
	UPROPERTY(EditDefaultsOnly, Category = "Cover")
	float EnterCoverDuration;

	/** Player's location when enter cover animation started. */
	FVector EnterCoverStartLocation;

	/** Target location for the player after entering cover animation. */
	FVector EnterCoverTargetLocation;

	/** Player's rotation when enter cover animation started. */
	FRotator EnterCoverStartRotation;

	/** Target rotation for the player after entering cover animation. */
	FRotator EnterCoverTargetRotation;

	/************************************************************************
	* Exit Cover Animation
	************************************************************************/
	/** True if player is currently exiting cover with animation. */
	bool bIsExitingCover;

	/** Time when the exit cover animation started. */
	float ExitCoverStartTime;

	/** Duration of the exit cover animation. */
	UPROPERTY(EditDefaultsOnly, Category = "Cover")
	float ExitCoverDuration;

	/** Player's location when exit cover animation started. */
	FVector ExitCoverStartLocation;

	/** Target location for the player after exiting cover animation. */
	FVector ExitCoverTargetLocation;

private:
    /** Timer handle for automatic firing */
    FTimerHandle TimerHandle_AutomaticFire;

	/** Flag to track if the dedicated aim button is pressed */
	bool bIsAiming;

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
};
