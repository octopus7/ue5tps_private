#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BoxPlayer.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class USpringArmComponent;
class UCameraComponent;
class USceneComponent;
class UNiagaraSystem;

/**
 * 기본 TPS 플레이어의 카메라/이동/점프만을 사용한 더미 박스 플레이어
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
    /** 카메라 붐: 플레이어 뒤에서 카메라를 배치 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    /** 팔로우 카메라 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    /** 기본 카메라 거리 */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float DefaultCameraArmLength;

    /** 기본 카메라 소켓 오프셋 */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    FVector DefaultCameraSocketOffset;

    /** 에임 시 카메라 거리 */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float AimingCameraArmLength;

    /** 에임 시 카메라 소켓 오프셋 */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    FVector AimingCameraSocketOffset;

    /** 카메라 보간 속도 */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float CameraInterpSpeed;

    /** 기본 입력 매핑 컨텍스트 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    /** 이동 입력 액션 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    /** 카메라 회전 입력 액션 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    /** 점프 입력 액션 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction;

    /** 에임 입력 액션 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* AimAction;

    /** 발사 입력 액션 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* FireAction;

    /** 기본 걷기 속도 */
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float DefaultWalkSpeed;

    /** 탄환 스폰 지점 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    USceneComponent* ProjectileSpawnPoint;

    /** 발사할 탄환 클래스 */
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    TSubclassOf<AActor> ProjectileClass;

    /** 발사 이펙트 */
    UPROPERTY(EditAnywhere, Category = "Combat")
    UNiagaraSystem* FireEffect;

    /** 카메라 에임 트레이스 거리 */
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float CameraAimTraceDistance;

    /** 발사 간격(자동 사격) */
    UPROPERTY(EditAnywhere, Category = "Combat")
    float TimeBetweenShots;

    /** 스폰할 무기 BP */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<AActor> WeaponBlueprint;

    /** 무기 장착 소켓 */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName WeaponSocketName;

    /** 스폰된 무기 참조 */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    AActor* SpawnedWeapon;

protected:
    /** 이동 입력 처리 */
    void Move(const FInputActionValue& Value);

    /** 카메라 회전 입력 처리 */
    void Look(const FInputActionValue& Value);

    /** 에임 시작 */
    void AimStarted();

    /** 에임 종료 */
    void AimStopped();

    /** 발사 시작 */
    void StartFire();

    /** 발사 종료 */
    void StopFire();

    /** 실제 발사 */
    void Fire();

    /** 카메라 기준 조준 방향 계산 */
    FVector CalculateCameraAimDirection(const FVector& MuzzleLocation, FVector& OutAimPoint);

    /** 카메라 파라미터 업데이트 */
    void UpdateCamera(float DeltaTime);

    /** 회전 플래그 적용 */
    void ApplyRotationSettings();

private:
    /** 자동사격 타이머 */
    FTimerHandle AutomaticFireHandle;

    /** 에임 여부 */
    bool bIsAiming;

public:
    /** CameraBoom 참조 */
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    /** FollowCamera 참조 */
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
