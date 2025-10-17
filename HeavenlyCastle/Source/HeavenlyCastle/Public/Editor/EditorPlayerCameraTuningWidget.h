#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Camera/PlayerCameraTuningTypes.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "EditorPlayerCameraTuningWidget.generated.h"

class UVerticalBox;
class UHorizontalBox;
class UTextBlock;
class UButton;
class UScrollBox;
class USpinBox;
class UCheckBox;

struct FCameraStateRowWidgets
{
    FName StateName = NAME_None;
    TWeakObjectPtr<UCheckBox> EnabledCheck;
    TWeakObjectPtr<USpinBox> ArmLengthSpin;
    TWeakObjectPtr<USpinBox> OffsetXSpin;
    TWeakObjectPtr<USpinBox> OffsetYSpin;
    TWeakObjectPtr<USpinBox> OffsetZSpin;
};

/**
 * BoxPlayer/TPSPlayer 카메라 튜닝을 위한 에디터 유틸리티 위젯
 */
UCLASS()
class HEAVENLYCASTLE_API UEditorPlayerCameraTuningWidget : public UEditorUtilityWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

protected:
    void BuildLayout();
    void RebuildStateRows(const FPlayerCameraTuningData& Data);
    void ClearStateRows();
    FPlayerCameraTuningData GatherDataFromUI() const;
    void RefreshFromSelection();
    void ApplyToSelection();
    TArray<AActor*> GetSelectedActors() const;
    void ApplyDataToActor(AActor* Actor, const FPlayerCameraTuningData& Data, bool bUpdateClassDefaults) const;

    UFUNCTION()
    void HandleRefreshButtonClicked();

    UFUNCTION()
    void HandleApplyButtonClicked();

private:
#if WITH_EDITORONLY_DATA
    UPROPERTY()
    UVerticalBox* RootBox = nullptr;

    UPROPERTY()
    UTextBlock* SelectionStatusText = nullptr;

    UPROPERTY()
    UScrollBox* StatesScrollBox = nullptr;

    UPROPERTY()
    USpinBox* InterpSpeedSpin = nullptr;

    UPROPERTY()
    UCheckBox* UpdateClassDefaultsCheck = nullptr;

    UPROPERTY()
    UButton* RefreshButton = nullptr;

    UPROPERTY()
    UButton* ApplyButton = nullptr;
#endif

    TArray<FCameraStateRowWidgets> StateRows;

    /** 현재 UI가 표현하고 있는 데이터. Refresh 시 업데이트 */
    FPlayerCameraTuningData CachedData;
};

#endif // WITH_EDITOR
