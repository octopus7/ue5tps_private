#pragma once

#include "CoreMinimal.h"
#include "PlayerCameraTuningTypes.generated.h"

/**
 * 개별 카메라 상태(예: Default, Aiming, Sprint 등)에 대한 튜닝 값
 */
USTRUCT(BlueprintType)
struct FPlayerCameraStateTuning
{
    GENERATED_BODY()

    /** 상태명. Default, Aiming, Sprint 등의 식별자로 사용 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FName StateName = NAME_None;

    /** SpringArm 의 TargetArmLength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float TargetArmLength = 400.f;

    /** SpringArm 의 SocketOffset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FVector SocketOffset = FVector::ZeroVector;

    /** 상태가 현재 적용 가능한지 여부 (UI 제어용) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bEnabled = true;
};

/**
 * 플레이어 캐릭터 한 명에 대한 전체 카메라 튜닝 패키지
 */
USTRUCT(BlueprintType)
struct FPlayerCameraTuningData
{
    GENERATED_BODY()

    /** 상태별 튜닝 값 목록 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    TArray<FPlayerCameraStateTuning> States;

    /** 카메라 보간 속도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraInterpSpeed = 20.f;
};

