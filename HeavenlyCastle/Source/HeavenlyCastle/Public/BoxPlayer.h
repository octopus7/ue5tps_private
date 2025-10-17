#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Cover/CoverTypes.h"
#include "Weapon/AmmoTypes.h"
#include "Weapon/AmmoConsumerInterface.h"
#include "Camera/CameraTuningInterface.h"
#include "BoxPlayer.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class USpringArmComponent;
class UCameraComponent;
class USceneComponent;
class UNiagaraSystem;
class UCoverControllerComponent;
class UCoverCameraComponent;
class UWidgetComponent;
class UGameStateWidget;

/**
 * 湲곕낯 TPS ?뚮젅?댁뼱??移대찓???대룞/?먰봽留뚯쓣 ?ъ슜???붾? 諛뺤뒪 ?뚮젅?댁뼱
 */
UCLASS()
class HEAVENLYCASTLE_API ABoxPlayer : public ACharacter, public IAmmoConsumerInterface, public ICameraTuningInterface
{
    GENERATED_BODY()

public:
    ABoxPlayer();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void Jump() override;
    virtual FPlayerCameraTuningData GetCameraTuningData_Implementation() const override;
    virtual void ApplyCameraTuningData_Implementation(const FPlayerCameraTuningData& NewData) override;
    virtual void RefreshCameraFromTuning_Implementation() override;

protected:
    /** 移대찓??遺? ?뚮젅?댁뼱 ?ㅼ뿉??移대찓?쇰? 諛곗튂 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    /** ?붾줈??移대찓??*/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    /** 而ㅻ쾭 而⑦듃濡ㅻ윭 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
    UCoverControllerComponent* CoverController;

    /** 而ㅻ쾭 移대찓???ы띁 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
    UCoverCameraComponent* CoverCamera;

    /** Player status widget */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
    UWidgetComponent* StatusWidgetComponent;

    /** 湲곕낯 移대찓??嫄곕━ */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float DefaultCameraArmLength;

    /** 湲곕낯 移대찓???뚯폆 ?ㅽ봽??*/
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    FVector DefaultCameraSocketOffset;

    /** ?먯엫 ??移대찓??嫄곕━ */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float AimingCameraArmLength;

    /** ?먯엫 ??移대찓???뚯폆 ?ㅽ봽??*/
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    FVector AimingCameraSocketOffset;

    /** 移대찓??蹂닿컙 ?띾룄 */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float CameraInterpSpeed;

    /** 湲곕낯 ?낅젰 留ㅽ븨 而⑦뀓?ㅽ듃 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    /** ?대룞 ?낅젰 ?≪뀡 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    /** 移대찓???뚯쟾 ?낅젰 ?≪뀡 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    /** ?먰봽 ?낅젰 ?≪뀡 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction;

    /** ?먯엫 ?낅젰 ?≪뀡 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* AimAction;

    /** 而ㅻ쾭 ?낅젰 ?≪뀡 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* CoverAction;

    /** Reload input action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* ReloadAction;

    /** 諛쒖궗 ?낅젰 ?≪뀡 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* FireAction;

    /** 湲곕낯 嫄룰린 ?띾룄 */
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float DefaultWalkSpeed;

    /** ?꾪솚 ?ㅽ룿 吏??*/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    USceneComponent* ProjectileSpawnPoint;

    /** 諛쒖궗???꾪솚 ?대옒??*/
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    TSubclassOf<AActor> ProjectileClass;

    /** 諛쒖궗 ?댄럺??*/
    UPROPERTY(EditAnywhere, Category = "Combat")
    UNiagaraSystem* FireEffect;

    /** 移대찓???먯엫 ?몃젅?댁뒪 嫄곕━ */
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float CameraAimTraceDistance;

    /** 諛쒖궗 媛꾧꺽(?먮룞 ?ш꺽) */
    UPROPERTY(EditAnywhere, Category = "Combat")
    float TimeBetweenShots;

    /** ?ㅽ룿??臾닿린 BP */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<AActor> WeaponBlueprint;

    /** 臾닿린 ?μ갑 ?뚯폆 */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName WeaponSocketName;

    /** Ammo configurations shared with TPS player */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo")
    TArray<FAmmoTypeConfig> AmmoTypeConfigs;

    /** Ammo type consumed by the current weapon (defaults to rifle). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo")
    EAmmoType EquippedAmmoType = EAmmoType::Rifle;

    /** ?ㅽ룿??臾닿린 李몄“ */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    AActor* SpawnedWeapon;

    /** 而ㅻ쾭 ?덉텧 ?꾧퀎媛?*/
    UPROPERTY(EditDefaultsOnly, Category = "Cover")
    float CoverExitForwardThreshold;

protected:
    /** ?대룞 ?낅젰 泥섎━ */
    void Move(const FInputActionValue& Value);

    /** 移대찓???뚯쟾 ?낅젰 泥섎━ */
    void Look(const FInputActionValue& Value);

    /** ?먯엫 ?쒖옉 */
    void AimStarted();

    /** ?먯엫 醫낅즺 */
    void AimStopped();

    /** 諛쒖궗 ?쒖옉 */
    void StartFire();

    /** 諛쒖궗 醫낅즺 */
    void StopFire();

    /** Reload input handler */
    void ReloadWeapon();

    virtual int32 AddAmmoToInventory_Implementation(EAmmoType AmmoType, int32 Amount) override;

    /** ?ㅼ젣 諛쒖궗 */
    void Fire();

    /** 移대찓??湲곗? 議곗? 諛⑺뼢 怨꾩궛 */
    FVector CalculateCameraAimDirection(const FVector& MuzzleLocation, FVector& OutAimPoint);

    /** 移대찓???뚮씪誘명꽣 ?낅뜲?댄듃 */
    void UpdateCamera(float DeltaTime);

    /** ?뚯쟾 ?뚮옒洹??곸슜 */
    void ApplyRotationSettings();

    /** 而ㅻ쾭 ?낅젰 泥섎━ */
    void HandleCoverAction();

    /** 而ㅻ쾭 ?щ? ?뺤씤 */
    bool IsInCover() const;

    /** Updates cover availability widget */
    void UpdateCoverAvailability();

    /** 而ㅻ쾭 ?곹깭 蹂寃?肄쒕갚 */
    UFUNCTION()
    void OnCoverStateChanged(ESimpleCoverState NewState);

    /** 而ㅻ쾭 移대찓???ㅽ봽??媛깆떊 */
    UFUNCTION()
    void OnCoverCameraOffsetUpdated(FVector Offset);

private:
    /** Game state widget instance */
    UPROPERTY()
    TObjectPtr<UGameStateWidget> GameStateWidgetInstance;

    /** Cover widget polling timer */
    FTimerHandle CoverWidgetUpdateHandle;

    /** ?먮룞?ш꺽 ??대㉧ */
    FTimerHandle AutomaticFireHandle;

    /** ?먯엫 ?щ? */
    bool bIsAiming;

    /** 而ㅻ쾭 移대찓???ㅽ봽??*/
    FVector CoverCameraOffset;

    /** Shared ammo runtime state */
    FAmmoInventory AmmoInventory;

    void UpdateAmmoUI();
    void HandleOutOfAmmo();
    bool ConsumeAmmo();

public:
    /** CameraBoom 李몄“ */
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    /** FollowCamera 李몄“ */
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};


