#include "SandCastleActor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

ASandCastleActor::ASandCastleActor()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    SetRootComponent(MeshComponent);
    MeshComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    MaxDurability = 900.f;
    Durability = MaxDurability;
    MidDamagedThresholdRatio = 0.66f;
    NearDestroyedThresholdRatio = 0.33f;
    CurrentState = ESandCastleState::Intact;
    bDestroyed = false;

    // HP text above head
    HPText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HPText"));
    HPText->SetupAttachment(MeshComponent);
    HPText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    HPText->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    HPText->SetWorldSize(108.f);
    HPText->SetTextRenderColor(FColor::White);
    HPText->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
    UpdateHPText();
}

void ASandCastleActor::BeginPlay()
{
    Super::BeginPlay();

    Durability = MaxDurability;

    Durability = FMath::Clamp(Durability, 0.f, MaxDurability);
    EvaluateAndApplyState(false);

    OnTakeAnyDamage.AddDynamic(this, &ASandCastleActor::HandleTakeAnyDamage);
}

void ASandCastleActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    // Ensure initial mesh matches current state in editor
    EvaluateAndApplyState(false);
}

void ASandCastleActor::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (bDestroyed || Damage <= 0.f)
    {
        return;
    }

    Durability = FMath::Clamp(Durability - Damage, 0.f, MaxDurability);

    UpdateHPText();

    if (Durability <= 0.f)
    {
        // Final destroy: spawn effect not attached, then destroy actor
        SpawnDestroyEffect();
        bDestroyed = true;
        Destroy();
        return;
    }

    // Evaluate new state and apply mesh; spawn stage effect if state changed
    EvaluateAndApplyState(true);

    // Play brief gold flash + shimmer feedback on damage
    PlayHitFlash();
}

void ASandCastleActor::EvaluateAndApplyState(bool bSpawnStageEffect)
{
    ESandCastleState NewState = ESandCastleState::Intact;
    const float Ratio = (MaxDurability > 0.f) ? (Durability / MaxDurability) : 0.f;

    if (Ratio <= NearDestroyedThresholdRatio)
    {
        NewState = ESandCastleState::NearDestroyed;
    }
    else if (Ratio <= MidDamagedThresholdRatio)
    {
        NewState = ESandCastleState::MidDamaged;
    }
    else
    {
        NewState = ESandCastleState::Intact;
    }

    if (NewState != CurrentState)
    {
        CurrentState = NewState;
        ApplyMeshForState(CurrentState);
        if (bSpawnStageEffect)
        {
            SpawnStageEffect();
        }
    }
    else
    {
        // Keep mesh consistent even if same state (e.g., edited in details)
        ApplyMeshForState(CurrentState);
    }
}

void ASandCastleActor::ApplyMeshForState(ESandCastleState NewState)
{
    if (!MeshComponent)
    {
        return;
    }

    UStaticMesh* DesiredMesh = nullptr;
    switch (NewState)
    {
    case ESandCastleState::Intact:        DesiredMesh = MeshIntact; break;
    case ESandCastleState::MidDamaged:    DesiredMesh = MeshMidDamaged; break;
    case ESandCastleState::NearDestroyed: DesiredMesh = MeshNearDestroyed; break;
    default: break;
    }

    if (DesiredMesh)
    {
        MeshComponent->SetStaticMesh(DesiredMesh);
    }
}

void ASandCastleActor::SpawnStageEffect()
{
    if (!StageChangeEffect)
    {
        return;
    }
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), StageChangeEffect, GetActorLocation(), GetActorRotation());
}

void ASandCastleActor::SpawnDestroyEffect()
{
    if (!DestroyEffect)
    {
        return;
    }
    // Not attached to this actor to ensure it persists after this actor is destroyed
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DestroyEffect, GetActorLocation(), GetActorRotation());
}

void ASandCastleActor::UpdateHPText()
{
    if (!HPText)
    {
        return;
    }
    const FString Txt = FString::Printf(TEXT("%.0f / %.0f"), Durability, MaxDurability);
    HPText->SetText(FText::FromString(Txt));
}

void ASandCastleActor::PlayHitFlash()
{
    if (!MeshComponent)
    {
        return;
    }

    // If a previous flash is running, end it first to restore materials cleanly
    EndHitFlash();

    bIsHitFlashing = true;
    HitFlashElapsed = 0.f;

    const int32 NumMats = MeshComponent->GetNumMaterials();
    OriginalMaterials.Reset();
    ActiveHitMIDs.Reset();

    if (HitFlashMaterial)
    {
        // Override all slots with the provided flash material
        OriginalMaterials.Reserve(NumMats);
        for (int32 Index = 0; Index < NumMats; ++Index)
        {
            OriginalMaterials.Add(MeshComponent->GetMaterial(Index));
            MeshComponent->SetMaterial(Index, HitFlashMaterial);
        }
    }
    else
    {
        // Parameter-based flash on current materials
        for (int32 Index = 0; Index < NumMats; ++Index)
        {
            UMaterialInstanceDynamic* MID = MeshComponent->CreateAndSetMaterialInstanceDynamic(Index);
            if (MID)
            {
                ActiveHitMIDs.Add(MID);
                MID->SetScalarParameterValue(ParamName_FlashIntensity, 1.0f);
                MID->SetVectorParameterValue(ParamName_FlashColor, HitFlashColor);
                MID->SetScalarParameterValue(ParamName_EmissiveBoost, HitFlashEmissive);
                MID->SetScalarParameterValue(ParamName_HitTime, 0.0f);
            }
        }
    }

    // Schedule shimmer updates and the end of flash
    if (UWorld* World = GetWorld())
    {
        // Shimmer tick ~60 FPS for the duration
        World->GetTimerManager().SetTimer(
            TimerHandle_TickHitFlash,
            this,
            &ASandCastleActor::TickHitFlash,
            1.0f / 60.0f,
            true
        );

        World->GetTimerManager().SetTimer(
            TimerHandle_EndHitFlash,
            this,
            &ASandCastleActor::EndHitFlash,
            HitFlashDuration,
            false
        );
    }
}

void ASandCastleActor::TickHitFlash()
{
    if (!bIsHitFlashing)
    {
        return;
    }

    HitFlashElapsed += 1.0f / 60.0f;

    // Ease-out the intensity over time
    const float Alpha = FMath::Clamp(1.0f - (HitFlashElapsed / FMath::Max(0.001f, HitFlashDuration)), 0.0f, 1.0f);

    // Update parameter-based MIDs
    for (UMaterialInstanceDynamic* MID : ActiveHitMIDs)
    {
        if (!MID) continue;
        MID->SetScalarParameterValue(ParamName_HitTime, HitFlashElapsed);
        MID->SetScalarParameterValue(ParamName_FlashIntensity, Alpha);
        MID->SetScalarParameterValue(ParamName_EmissiveBoost, HitFlashEmissive * Alpha);
    }
}

void ASandCastleActor::EndHitFlash()
{
    if (!bIsHitFlashing)
    {
        return;
    }

    bIsHitFlashing = false;

    // Stop timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TimerHandle_TickHitFlash);
        World->GetTimerManager().ClearTimer(TimerHandle_EndHitFlash);
    }

    // Restore original materials if we overrode them
    if (OriginalMaterials.Num() > 0 && MeshComponent)
    {
        const int32 NumMats = MeshComponent->GetNumMaterials();
        for (int32 Index = 0; Index < NumMats && Index < OriginalMaterials.Num(); ++Index)
        {
            MeshComponent->SetMaterial(Index, OriginalMaterials[Index]);
        }
        OriginalMaterials.Reset();
    }

    // Or, turn off the parameters on dynamic instances
    for (UMaterialInstanceDynamic* MID : ActiveHitMIDs)
    {
        if (!MID) continue;
        MID->SetScalarParameterValue(ParamName_FlashIntensity, 0.0f);
        MID->SetScalarParameterValue(ParamName_EmissiveBoost, 0.0f);
    }
    ActiveHitMIDs.Reset();
}
