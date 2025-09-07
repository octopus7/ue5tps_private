// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponMicroUzi.generated.h"

class UStaticMeshComponent;
class UNiagaraSystem;

UCLASS()
class HEAVENLYCASTLE_API AWeaponMicroUzi : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponMicroUzi();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Weapon Mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WeaponMesh;

	/** Niagara FX for Muzzle Flash */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* MuzzleFlashEffect;

	/** Niagara FX for Eject Casing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* EjectCasingEffect;

	/** Name of the muzzle socket on the weapon mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets")
	FName MuzzleSocketName;

	/** Name of the casing eject socket on the weapon mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets")
	FName CasingEjectSocketName;

	/** Function to call when firing the weapon */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire();

};
