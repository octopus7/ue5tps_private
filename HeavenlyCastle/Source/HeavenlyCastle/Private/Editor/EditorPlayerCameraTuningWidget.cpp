#include "Editor/EditorPlayerCameraTuningWidget.h"

#if WITH_EDITOR

#include "Camera/CameraTuningInterface.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SpinBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "Internationalization/Text.h"
#include "ScopedTransaction.h"
#include "Blueprint/WidgetTree.h"
#include "Styling/SlateTypes.h"

namespace UEPlayerCameraTuningWidget
{
    static const FName DefaultStateName(TEXT("Default"));
    static const FName AimingStateName(TEXT("Aiming"));
    static const FName SprintStateName(TEXT("Sprint"));
}

void UEditorPlayerCameraTuningWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BuildLayout();
    RefreshFromSelection();
}

void UEditorPlayerCameraTuningWidget::NativeDestruct()
{
    ClearStateRows();
    Super::NativeDestruct();
}

void UEditorPlayerCameraTuningWidget::BuildLayout()
{
    if (!WidgetTree)
    {
        return;
    }

    WidgetTree->RootWidget = nullptr;
    StateRows.Reset();

    RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RootBox"));
    WidgetTree->RootWidget = RootBox;

    if (!RootBox)
    {
        return;
    }

    // 안내 문구
    SelectionStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SelectionStatus"));
    if (SelectionStatusText)
    {
        SelectionStatusText->SetText(FText::FromString(TEXT("선택된 플레이어 없음")));
        RootBox->AddChildToVerticalBox(SelectionStatusText);
    }

    // 버튼 행
    UHorizontalBox* ButtonRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ButtonRow"));
    if (ButtonRow)
    {
        RootBox->AddChildToVerticalBox(ButtonRow);

        RefreshButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RefreshButton"));
        if (RefreshButton)
        {
            UTextBlock* RefreshLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RefreshLabel"));
            if (RefreshLabel)
            {
                RefreshLabel->SetText(FText::FromString(TEXT("선택 불러오기")));
                RefreshButton->AddChild(RefreshLabel);
            }
            RefreshButton->OnClicked.AddDynamic(this, &UEditorPlayerCameraTuningWidget::HandleRefreshButtonClicked);
            ButtonRow->AddChildToHorizontalBox(RefreshButton);
        }

        ApplyButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ApplyButton"));
        if (ApplyButton)
        {
            UTextBlock* ApplyLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ApplyLabel"));
            if (ApplyLabel)
            {
                ApplyLabel->SetText(FText::FromString(TEXT("선택 항목 적용")));
                ApplyButton->AddChild(ApplyLabel);
            }
            ApplyButton->OnClicked.AddDynamic(this, &UEditorPlayerCameraTuningWidget::HandleApplyButtonClicked);
            ButtonRow->AddChildToHorizontalBox(ApplyButton);
        }
    }

    // 인터프 속도 행
    UHorizontalBox* InterpRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("InterpRow"));
    if (InterpRow)
    {
        RootBox->AddChildToVerticalBox(InterpRow);

        UTextBlock* InterpLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("InterpLabel"));
        if (InterpLabel)
        {
            InterpLabel->SetText(FText::FromString(TEXT("Camera Interp Speed")));
            UHorizontalBoxSlot* LabelSlot = InterpRow->AddChildToHorizontalBox(InterpLabel);
            if (LabelSlot)
            {
                LabelSlot->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
                LabelSlot->SetVerticalAlignment(VAlign_Center);
            }
        }

        InterpSpeedSpin = WidgetTree->ConstructWidget<USpinBox>(USpinBox::StaticClass(), TEXT("InterpSpeedSpin"));
        if (InterpSpeedSpin)
        {
            InterpSpeedSpin->SetMinValue(0.f);
            InterpSpeedSpin->SetMaxValue(120.f);
            InterpSpeedSpin->SetValue(20.f);
            InterpSpeedSpin->SetDelta(0.5f);
            UHorizontalBoxSlot* SpinSlot = InterpRow->AddChildToHorizontalBox(InterpSpeedSpin);
            if (SpinSlot)
            {
                SpinSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            }
        }
    }

    // 클래스 기본값 체크
    UHorizontalBox* DefaultsRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("DefaultsRow"));
    if (DefaultsRow)
    {
        RootBox->AddChildToVerticalBox(DefaultsRow);

        UpdateClassDefaultsCheck = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), TEXT("UpdateClassDefaultsCheck"));
        if (UpdateClassDefaultsCheck)
        {
            UpdateClassDefaultsCheck->SetIsChecked(true);

            UTextBlock* DefaultsLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DefaultsLabel"));
            if (DefaultsLabel)
            {
                DefaultsLabel->SetText(FText::FromString(TEXT("선택된 클래스 기본값도 갱신")));
                UpdateClassDefaultsCheck->AddChild(DefaultsLabel);
            }

            DefaultsRow->AddChildToHorizontalBox(UpdateClassDefaultsCheck);
        }
    }

    // 상태 스크롤 박스
    StatesScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("StatesScrollBox"));
    if (StatesScrollBox)
    {
        RootBox->AddChildToVerticalBox(StatesScrollBox);
    }
}

void UEditorPlayerCameraTuningWidget::RebuildStateRows(const FPlayerCameraTuningData& Data)
{
    ClearStateRows();
    CachedData = Data;

    if (!StatesScrollBox || !WidgetTree)
    {
        return;
    }

    for (const FPlayerCameraStateTuning& State : Data.States)
    {
        UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
        if (!Row)
        {
            continue;
        }

        UCheckBox* EnabledCheckWidget = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
        if (EnabledCheckWidget)
        {
            EnabledCheckWidget->SetIsChecked(State.bEnabled);

            UTextBlock* CheckLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
            if (CheckLabel)
            {
                CheckLabel->SetText(FText::FromName(State.StateName));
                EnabledCheckWidget->AddChild(CheckLabel);
            }

            UHorizontalBoxSlot* EnabledSlot = Row->AddChildToHorizontalBox(EnabledCheckWidget);
            if (EnabledSlot)
            {
                EnabledSlot->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
                EnabledSlot->SetVerticalAlignment(VAlign_Center);
            }
        }

        auto MakeSpinBox = [this, Row](const FString& Name, float Value) -> USpinBox*
        {
            USpinBox* Spin = WidgetTree->ConstructWidget<USpinBox>(USpinBox::StaticClass(), FName(*Name));
            if (Spin && Row)
            {
                Spin->SetMinSliderValue(-2000.f);
                Spin->SetMaxSliderValue(2000.f);
                Spin->SetMinValue(-10000.f);
                Spin->SetMaxValue(10000.f);
                Spin->SetDelta(1.f);
                Spin->SetValue(Value);
            if (UHorizontalBoxSlot* SpinSlot = Row->AddChildToHorizontalBox(Spin))
            {
                SpinSlot->SetPadding(FMargin(0.f, 0.f, 4.f, 0.f));
                SpinSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            }
            }
            return Spin;
        };

        USpinBox* ArmLengthSpinWidget = MakeSpinBox(State.StateName.ToString() + TEXT("_ArmLength"), State.TargetArmLength);
        USpinBox* OffsetXSpinWidget = MakeSpinBox(State.StateName.ToString() + TEXT("_OffsetX"), State.SocketOffset.X);
        USpinBox* OffsetYSpinWidget = MakeSpinBox(State.StateName.ToString() + TEXT("_OffsetY"), State.SocketOffset.Y);
        USpinBox* OffsetZSpinWidget = MakeSpinBox(State.StateName.ToString() + TEXT("_OffsetZ"), State.SocketOffset.Z);

        StatesScrollBox->AddChild(Row);

        FCameraStateRowWidgets RowWidgets;
        RowWidgets.StateName = State.StateName;
        RowWidgets.EnabledCheck = EnabledCheckWidget;
        RowWidgets.ArmLengthSpin = ArmLengthSpinWidget;
        RowWidgets.OffsetXSpin = OffsetXSpinWidget;
        RowWidgets.OffsetYSpin = OffsetYSpinWidget;
        RowWidgets.OffsetZSpin = OffsetZSpinWidget;

        StateRows.Add(RowWidgets);
    }
}

void UEditorPlayerCameraTuningWidget::ClearStateRows()
{
    StateRows.Reset();
    if (StatesScrollBox)
    {
        StatesScrollBox->ClearChildren();
    }
}

FPlayerCameraTuningData UEditorPlayerCameraTuningWidget::GatherDataFromUI() const
{
    FPlayerCameraTuningData Data;

    if (InterpSpeedSpin)
    {
        Data.CameraInterpSpeed = InterpSpeedSpin->GetValue();
    }

    for (const FCameraStateRowWidgets& Row : StateRows)
    {
        FPlayerCameraStateTuning State;
        State.StateName = Row.StateName;
        if (UCheckBox* EnabledCheck = Row.EnabledCheck.Get())
        {
            State.bEnabled = EnabledCheck->IsChecked();
        }
        else
        {
            State.bEnabled = true;
        }

        if (USpinBox* ArmLengthSpin = Row.ArmLengthSpin.Get())
        {
            State.TargetArmLength = ArmLengthSpin->GetValue();
        }

        float OffsetX = 0.f;
        if (USpinBox* OffsetXSpin = Row.OffsetXSpin.Get())
        {
            OffsetX = OffsetXSpin->GetValue();
        }

        float OffsetY = 0.f;
        if (USpinBox* OffsetYSpin = Row.OffsetYSpin.Get())
        {
            OffsetY = OffsetYSpin->GetValue();
        }

        float OffsetZ = 0.f;
        if (USpinBox* OffsetZSpin = Row.OffsetZSpin.Get())
        {
            OffsetZ = OffsetZSpin->GetValue();
        }
        State.SocketOffset = FVector(OffsetX, OffsetY, OffsetZ);

        Data.States.Add(State);
    }

    return Data;
}

void UEditorPlayerCameraTuningWidget::RefreshFromSelection()
{
    const TArray<AActor*> Actors = GetSelectedActors();
    const int32 TotalCount = Actors.Num();

    if (SelectionStatusText)
    {
        if (TotalCount == 0)
        {
            SelectionStatusText->SetText(FText::FromString(TEXT("선택된 플레이어 없음")));
        }
        else
        {
            const FString Label = FString::Printf(TEXT("선택된 액터: %d개"), TotalCount);
            SelectionStatusText->SetText(FText::FromString(Label));
        }
    }

    for (AActor* Actor : Actors)
    {
        if (!Actor || !Actor->GetClass()->ImplementsInterface(UCameraTuningInterface::StaticClass()))
        {
            continue;
        }

        FPlayerCameraTuningData Data = ICameraTuningInterface::Execute_GetCameraTuningData(Actor);
        if (InterpSpeedSpin)
        {
            InterpSpeedSpin->SetValue(Data.CameraInterpSpeed);
        }
        RebuildStateRows(Data);
        return;
    }

    // 지원하는 액터가 없으면 초기화
    ClearStateRows();
}

void UEditorPlayerCameraTuningWidget::ApplyToSelection()
{
    const TArray<AActor*> Actors = GetSelectedActors();
    if (Actors.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("카메라 튜닝을 적용할 액터가 선택되어 있지 않습니다."));
        return;
    }

    const FPlayerCameraTuningData Data = GatherDataFromUI();
    const bool bUpdateClassDefaults = UpdateClassDefaultsCheck ? UpdateClassDefaultsCheck->IsChecked() : false;

    FScopedTransaction Transaction(NSLOCTEXT("EditorCameraTuning", "ApplyCameraTuning", "Apply Player Camera Tuning"));

    for (AActor* Actor : Actors)
    {
        if (!Actor || !Actor->GetClass()->ImplementsInterface(UCameraTuningInterface::StaticClass()))
        {
            continue;
        }

        Actor->Modify();
        ApplyDataToActor(Actor, Data, bUpdateClassDefaults);
    }
}

TArray<AActor*> UEditorPlayerCameraTuningWidget::GetSelectedActors() const
{
    TArray<AActor*> Result;
    if (GEditor)
    {
        if (USelection* SelectedActors = GEditor->GetSelectedActors())
        {
            SelectedActors->GetSelectedObjects(Result);
        }
    }
    return Result;
}

void UEditorPlayerCameraTuningWidget::ApplyDataToActor(AActor* Actor, const FPlayerCameraTuningData& Data, bool bUpdateClassDefaults) const
{
    if (!Actor)
    {
        return;
    }

    ICameraTuningInterface::Execute_ApplyCameraTuningData(Actor, Data);
    ICameraTuningInterface::Execute_RefreshCameraFromTuning(Actor);

    if (bUpdateClassDefaults)
    {
        if (UObject* CDO = Actor->GetClass()->GetDefaultObject())
        {
            if (CDO->GetClass()->ImplementsInterface(UCameraTuningInterface::StaticClass()))
            {
                ICameraTuningInterface::Execute_ApplyCameraTuningData(CDO, Data);
            }
        }
    }
}

void UEditorPlayerCameraTuningWidget::HandleRefreshButtonClicked()
{
    RefreshFromSelection();
}

void UEditorPlayerCameraTuningWidget::HandleApplyButtonClicked()
{
    ApplyToSelection();
}

#endif // WITH_EDITOR
