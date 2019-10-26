// Fill out your copyright notice in the Description page of Project Settings.

#include "TPSWeapon.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h" //for damage and particle effects
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"

//For debug draw purposes
int32 DebugDrawWeapons = 0;
//FAutoConsoleVariableRef takes a text, a reference variable, a tooltip, and an event in its constructor
FAutoConsoleVariableRef VCARDrawWeapons = FAutoConsoleVariableRef(
												TEXT("TPS.DebugDrawWeapons"), DebugDrawWeapons, 
												TEXT("Draws Debug Line Trace"), ECVF_Cheat);

// Sets default values
ATPSWeapon::ATPSWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh Component"));
	RootComponent = (USceneComponent*)MeshComp;

	TrailEffectParameter = "BeamEnd";
}

// Called when the game starts or when spawned
void ATPSWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATPSWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATPSWeapon::Fire() {

	//create an owner for the weapon
	AActor* MyOwner = GetOwner();

	if (MyOwner) {
		//LineTracing requires a range (length)
		FVector EyeLocation;
		FRotator EyeRotation; //FRotators are, basically, quaternions
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector destination = EyeLocation + 10000 * EyeRotation.Vector();
		FVector TrailEnd = destination;

		FHitResult HitResult;
		
		if (GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, destination, ECC_Visibility)) {
			
			AActor* victim = HitResult.GetActor();
			UGameplayStatics::ApplyPointDamage(victim, 20.0F, EyeRotation.Vector(), HitResult, MyOwner->GetInstigatorController(), this, damageType);			
			
			TrailEnd = HitResult.ImpactPoint; //gotta read smoke trail end
			if (ImpactEffectBlood) {
				//what happens when it hits. Different effects go here
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffectBlood , HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
			}
		}

		FVector MuzzlePosition = MeshComp->GetSocketLocation(MuzzleSocket);

		if (MuzzleEffect) {
			//fire from muzzle
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, (USceneComponent*)MeshComp, MuzzleSocket, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);
		}

		if (TrailEffect) {
			//smoke trail
			UParticleSystemComponent* TrailComp =  UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TrailEffect, MuzzlePosition);

			if (TrailComp) {
				TrailComp->SetVectorParameter(TrailEffectParameter, TrailEnd);
			}
		}

		if (DebugDrawWeapons > 0) {
			DrawDebugLine(GetWorld(), EyeLocation, destination, FColor::Cyan, false, 1.0, 0, 2.5f);
		}
		
	}
	
}

void ATPSWeapon::StartFire()
{
}

void ATPSWeapon::EndFire()
{
}
