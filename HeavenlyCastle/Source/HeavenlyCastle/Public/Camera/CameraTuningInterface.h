#pragma once

#include "UObject/Interface.h"
#include "PlayerCameraTuningTypes.h"
#include "CameraTuningInterface.generated.h"

UINTERFACE(Blueprintable)
class HEAVENLYCASTLE_API UCameraTuningInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * BoxPlayer, TPSPlayer 모두가 구현하여 에디터 유틸리티 위젯에서 공통으로 제어하기 위한 인터페이스
 */
class HEAVENLYCASTLE_API ICameraTuningInterface
{
    GENERATED_BODY()

public:
    /** 현재 캐릭터가 지원하는 카메라 상태 목록과 값을 반환 */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera|Tuning")
    FPlayerCameraTuningData GetCameraTuningData() const;

    /** 위젯에서 받은 튜닝 값을 적용 */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera|Tuning")
    void ApplyCameraTuningData(const FPlayerCameraTuningData& NewData);

    /** 현재 상태가 에디터에서 즉시 반영되도록 강제 갱신 */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera|Tuning")
    void RefreshCameraFromTuning();
};

