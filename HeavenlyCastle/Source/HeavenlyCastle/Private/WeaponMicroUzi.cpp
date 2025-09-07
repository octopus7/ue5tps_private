// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponMicroUzi.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

// Sets default values
AWeaponMicroUzi::AWeaponMicroUzi()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create and set up the weapon mesh
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;

	// Set default socket names
	MuzzleSocketName = TEXT("Muzzle"); // Assuming a socket named 'Muzzle' exists on your static mesh
	CasingEjectSocketName = TEXT("CasingEject"); // Assuming a socket named 'CasingEject' exists on your static mesh
}

// Called when the game starts or when spawned
void AWeaponMicroUzi::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponMicroUzi::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponMicroUzi::Fire()
{
	// Spawn Muzzle Flash Effect
	if (MuzzleFlashEffect && WeaponMesh->DoesSocketExist(MuzzleSocketName))
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			MuzzleFlashEffect,
			WeaponMesh,
			MuzzleSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	// Spawn Casing Eject Effect
	if (EjectCasingEffect && WeaponMesh->DoesSocketExist(CasingEjectSocketName))
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			EjectCasingEffect,
			WeaponMesh,
			CasingEjectSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}
}
