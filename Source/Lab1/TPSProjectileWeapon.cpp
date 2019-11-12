// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSProjectileWeapon.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Grenade.h"

ATPSProjectileWeapon::ATPSProjectileWeapon() {
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = (USceneComponent*)MeshComp;
}

void ATPSProjectileWeapon::BeginPlay() {
	Super::BeginPlay();
}

void ATPSProjectileWeapon::Fire() {

	APawn* MyOwner = Cast<APawn>(GetOwner());

	if (MyOwner) {

		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		//100(cm) should be enough
		FVector LineEnd = EyeLocation + 100.f * EyeRotation.Vector();
		FVector TrailEnd = LineEnd;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.bTraceComplex = false;
		QueryParams.bReturnPhysicalMaterial = false;

		if (MuzzleEffect) {

			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocket);
		}

		FVector MuzzlePosition = MeshComp->GetSocketLocation(MuzzleSocket);
		if (TrailEffect) {

			UParticleSystemComponent* TrailComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TrailEffect, MuzzlePosition);

			if (TrailComp) {
				//opted for a single 1m trail
				TrailComp->SetVectorParameter(TrailEffectParameter, LineEnd);
			}
		}

		FActorSpawnParameters spawnParams;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		spawnParams.Instigator = MyOwner;
		//spawn grenade
		Grenade = GetWorld()->SpawnActor<AGrenade>(Projectile, MuzzlePosition, EyeRotation, spawnParams);
		Grenade->MeshComp->AddImpulse(LaunchForce * EyeRotation.Vector());

		APlayerController* PlayerController = Cast<APlayerController>(MyOwner->GetController());
		if (PlayerController) {

			PlayerController->ClientPlayCameraShake(FireCameraShake);
		}
	}	
}

void ATPSProjectileWeapon::Tick(float deltaSeconds) {
	Super::Tick(deltaSeconds);
}