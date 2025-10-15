#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BoxPlayer.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class USpringArmComponent;
class UCameraComponent;

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

    /** 기본 걷기 속도 */
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float DefaultWalkSpeed;

protected:
    /** 이동 입력 처리 */
    void Move(const FInputActionValue& Value);

    /** 카메라 회전 입력 처리 */
    void Look(const FInputActionValue& Value);

public:
    /** CameraBoom 참조 */
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    /** FollowCamera 참조 */
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
