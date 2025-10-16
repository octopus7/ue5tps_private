#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Cover/CoverTypes.h"
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

/**
 * 기본 TPS ?�레?�어??카메???�동/?�프만을 ?�용???��? 박스 ?�레?�어
 */
UCLASS()
class HEAVENLYCASTLE_API ABoxPlayer : public ACharacter
{
    GENERATED_BODY()

public:
    ABoxPlayer();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void Jump() override;

protected:
    /** 카메??�? ?�레?�어 ?�에??카메?��? 배치 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    /** ?�로??카메??*/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    /** 커버 컨트롤러 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
    UCoverControllerComponent* CoverController;

    /** 커버 카메???�퍼 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
    UCoverCameraComponent* CoverCamera;

    /** 기본 카메??거리 */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float DefaultCameraArmLength;

    /** 기본 카메???�켓 ?�프??*/
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    FVector DefaultCameraSocketOffset;

    /** ?�임 ??카메??거리 */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float AimingCameraArmLength;

    /** ?�임 ??카메???�켓 ?�프??*/
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    FVector AimingCameraSocketOffset;

    /** 카메??보간 ?�도 */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float CameraInterpSpeed;

    /** 기본 ?�력 매핑 컨텍?�트 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    /** ?�동 ?�력 ?�션 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    /** 카메???�전 ?�력 ?�션 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    /** ?�프 ?�력 ?�션 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction;

    /** ?�임 ?�력 ?�션 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* AimAction;

    /** 커버 ?�력 ?�션 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* CoverAction;

    /** 발사 ?�력 ?�션 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* FireAction;

    /** 기본 걷기 ?�도 */
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float DefaultWalkSpeed;

    /** ?�환 ?�폰 지??*/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    USceneComponent* ProjectileSpawnPoint;

    /** 발사???�환 ?�래??*/
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    TSubclassOf<AActor> ProjectileClass;

    /** 발사 ?�펙??*/
    UPROPERTY(EditAnywhere, Category = "Combat")
    UNiagaraSystem* FireEffect;

    /** 카메???�임 ?�레?�스 거리 */
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float CameraAimTraceDistance;

    /** 발사 간격(?�동 ?�격) */
    UPROPERTY(EditAnywhere, Category = "Combat")
    float TimeBetweenShots;

    /** ?�폰??무기 BP */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<AActor> WeaponBlueprint;

    /** 무기 ?�착 ?�켓 */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName WeaponSocketName;

    /** ?�폰??무기 참조 */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    AActor* SpawnedWeapon;

    /** 커버 ?�출 ?�계�?*/
    UPROPERTY(EditDefaultsOnly, Category = "Cover")
    float CoverExitForwardThreshold;

protected:
    /** ?�동 ?�력 처리 */
    void Move(const FInputActionValue& Value);

    /** 카메???�전 ?�력 처리 */
    void Look(const FInputActionValue& Value);

    /** ?�임 ?�작 */
    void AimStarted();

    /** ?�임 종료 */
    void AimStopped();

    /** 발사 ?�작 */
    void StartFire();

    /** 발사 종료 */
    void StopFire();

    /** ?�제 발사 */
    void Fire();

    /** 카메??기�? 조�? 방향 계산 */
    FVector CalculateCameraAimDirection(const FVector& MuzzleLocation, FVector& OutAimPoint);

    /** 카메???�라미터 ?�데?�트 */
    void UpdateCamera(float DeltaTime);

    /** ?�전 ?�래�??�용 */
    void ApplyRotationSettings();

    /** 커버 ?�력 처리 */
    void HandleCoverAction();

    /** 커버 ?��? ?�인 */
    bool IsInCover() const;

    /** 커버 ?�태 변�?콜백 */
    UFUNCTION()
    void OnCoverStateChanged(ESimpleCoverState NewState);

    /** 커버 카메???�프??갱신 */
    UFUNCTION()
    void OnCoverCameraOffsetUpdated(FVector Offset);

private:
    /** ?�동?�격 ?�?�머 */
    FTimerHandle AutomaticFireHandle;

    /** ?�임 ?��? */
    bool bIsAiming;

    /** 커버 카메???�프??*/
    FVector CoverCameraOffset;

public:
    /** CameraBoom 참조 */
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    /** FollowCamera 참조 */
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

