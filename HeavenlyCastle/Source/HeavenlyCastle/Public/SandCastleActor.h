#pragma once

#include "CoreMinimal.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Actor.h"
#include "SandCastleActor.generated.h"

class UNiagaraSystem;
class UUserWidget;
class UWidgetComponent;

UENUM(BlueprintType)
enum class ESandCastleState : uint8
{
    Intact          UMETA(DisplayName = "Intact"),
    MidDamaged      UMETA(DisplayName = "MidDamaged"),
    NearDestroyed   UMETA(DisplayName = "NearDestroyed"),
};

UCLASS()
class HEAVENLYCASTLE_API ASandCastleActor : public AActor
{
    GENERATED_BODY()

public:
    ASandCastleActor();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* HPText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|Mesh")
    class UStaticMesh* MeshIntact;

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|Mesh")
    class UStaticMesh* MeshMidDamaged;

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|Mesh")
    class UStaticMesh* MeshNearDestroyed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Destructible|Durability")
    float MaxDurability;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destructible|Durability")
    float Durability;

    // Remaining ratio thresholds (Durability/MaxDurability)
    UPROPERTY(EditDefaultsOnly, Category = "Destructible|Durability", meta=(ClampMin="0.0", ClampMax="1.0"))
    float MidDamagedThresholdRatio;      // e.g. 0.66 -> switch to mid-damaged when remaining <= 66%

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|Durability", meta=(ClampMin="0.0", ClampMax="1.0"))
    float NearDestroyedThresholdRatio;   // e.g. 0.33 -> switch to near-destroyed when remaining <= 33%

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|FX")
    UNiagaraSystem* StageChangeEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|FX")
    UNiagaraSystem* DestroyEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destructible|State")
    ESandCastleState CurrentState;

    bool bDestroyed;

protected:
    UFUNCTION()
    void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    void EvaluateAndApplyState(bool bSpawnStageEffect);
    void ApplyMeshForState(ESandCastleState NewState);
    void SpawnStageEffect();
    void SpawnDestroyEffect();
    void UpdateHPText();

    /************************************************************************
    * Hit Flash (Gold shimmer on damage)
    ************************************************************************/
protected:
    /** Optional override material used briefly when hit (gold + shimmer). If unset, tries parameter-based flash on current materials. */
    UPROPERTY(EditDefaultsOnly, Category = "Destructible|FX|HitFlash")
    class UMaterialInterface* HitFlashMaterial;

    /** Duration of the flash/shimmer in seconds. */
    UPROPERTY(EditDefaultsOnly, Category = "Destructible|FX|HitFlash", meta=(ClampMin="0.01", UIMin="0.05", UIMax="1.0"))
    float HitFlashDuration = 1 / 30.f;

    /** Gold tint color used when parameter-based flashing (if material supports). */
    UPROPERTY(EditDefaultsOnly, Category = "Destructible|FX|HitFlash")
    FLinearColor HitFlashColor = FLinearColor(1.0f, 0.85f, 0.2f, 1.0f);

    /** Emissive boost while flashing (parameter-based). */
    UPROPERTY(EditDefaultsOnly, Category = "Destructible|FX|HitFlash")
    float HitFlashEmissive = 15.0f;

    /** Parameter names expected on the base material (when not using HitFlashMaterial). */
    UPROPERTY(EditDefaultsOnly, Category = "Destructible|FX|HitFlash")
    FName ParamName_FlashIntensity = TEXT("HitFlash");

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|FX|HitFlash")
    FName ParamName_FlashColor = TEXT("HitColor");

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|FX|HitFlash")
    FName ParamName_EmissiveBoost = TEXT("EmissiveBoost");

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|FX|HitFlash")
    FName ParamName_HitTime = TEXT("HitTime");

private:
    /** Original materials cache when overriding with HitFlashMaterial. */
    UPROPERTY(Transient)
    TArray<class UMaterialInterface*> OriginalMaterials;

    /** Active MIDs when doing parameter-based flash. */
    UPROPERTY(Transient)
    TArray<class UMaterialInstanceDynamic*> ActiveHitMIDs;

    /** Timers driving the flash. */
    FTimerHandle TimerHandle_EndHitFlash;
    FTimerHandle TimerHandle_TickHitFlash;

    /** State for animated shimmer. */
    float HitFlashElapsed = 0.f;
    bool bIsHitFlashing = false;

    void PlayHitFlash();
    void EndHitFlash();
    void TickHitFlash();
};
