// Simple teleport portal actor

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/WeakObjectPtr.h"
#include "Portal.generated.h"

class UBoxComponent;
class UTextRenderComponent;

UCLASS()
class HEAVENLYCASTLE_API APortal : public AActor
{
	GENERATED_BODY()

public:
    APortal();

protected:
	virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaSeconds) override;

    virtual void OnConstruction(const FTransform& Transform) override;

protected:
	/** Trigger volume used to detect entering actors */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
    UBoxComponent* TriggerBox;

    /** Text label shown above the portal displaying "PortalId -> TargetPortalId" */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    UTextRenderComponent* TextLabel;

	/** Unique identifier for this portal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FString PortalId;

	/** Id of the portal to teleport to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FString TargetPortalId;

	/** Offset applied at destination relative to target portal's forward vector */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	float DestinationForwardOffset;

	/** Optional vertical offset when arriving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	float DestinationZOffset;

	/** Draw debug lines for the trigger box */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugDraw;

	/** Thickness of debug lines */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugLineThickness;

    /** Height above actor origin for the label */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float LabelHeight;

    /** Text world size for the label */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float LabelWorldSize;

    /** Rotate label to face the camera every tick */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bFaceCamera;

    /** If true, only rotate around Z (yaw) to keep text upright */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bFaceCameraYawOnly;

    /** Smoothly interpolate label rotation toward camera */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bSmoothFaceCamera;

    /** Interp speed for smooth facing (deg/sec-ish). Higher = snappier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float FaceCameraInterpSpeed;

    /** Enable distance-based label visibility control */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bUseDistanceVisibility;

    /** Label visible when camera distance is within [Min, Max] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float LabelVisibleMinDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float LabelVisibleMaxDistance;

private:
    UFUNCTION()
    void OnTriggerBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    APortal* FindDestinationPortal() const;

    void UpdateLabelText();

    // Original trigger box extent captured at BeginPlay (local/unscaled)
    FVector OriginalBoxExtent;

    /** Seconds to disable teleporting after a teleport (prevents ping-pong). Default 3s */
    UPROPERTY(EditAnywhere, Category = "Portal")
    float TeleportCooldownSeconds;

    // World time when this portal can teleport again
    double CooldownEndTime = 0.0;

    bool IsOnCooldown() const;
    void StartCooldown();

    /** Use per-actor cooldown instead of global portal cooldown */
    UPROPERTY(EditAnywhere, Category = "Portal")
    bool bUseActorCooldown = true;

    /** Also allow global portal cooldown (applies to everyone) */
    UPROPERTY(EditAnywhere, Category = "Portal")
    bool bUseGlobalCooldown = false;

    // Records when each actor can use this portal again
    TMap<TWeakObjectPtr<AActor>, double> ActorCooldowns;

    bool IsActorOnCooldown(AActor* Actor);
    void StartActorCooldown(AActor* Actor);
    void PruneActorCooldowns();
};
