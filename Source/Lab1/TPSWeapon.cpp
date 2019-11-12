// Fill out your copyright notice in the Description page of Project Settings.

#include "TPSWeapon.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h" //for damage and particle effects
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Lab1.h"

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

	BaseDamage = 20.0f;
	DamageMultiplier = 4.0f;
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

	//create an owner for the weapon Originally, it was an Actor, hence the casting
	APawn* MyOwner = Cast<APawn>(GetOwner());

	if (MyOwner) {
		//LineTracing requires a range (length)
		FVector EyeLocation;
		FRotator EyeRotation; //FRotators are, basically, quaternions
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector destination = EyeLocation + 10000 * EyeRotation.Vector();
		FVector TrailEnd = destination;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = false;
		QueryParams.bReturnPhysicalMaterial = true;

		FHitResult HitResult;
		
		if (GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, destination, WeaponTraceChannel, QueryParams)) {
			
			AActor* victim = HitResult.GetActor();

			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Cast<UPhysicalMaterial>(HitResult.PhysMaterial));

			UParticleSystem* ImpactEffectToPlay = NULL;
			float DamageToApply = BaseDamage;

			switch (SurfaceType) {
				case Flash_Default:
					ImpactEffectToPlay = ImpactEffectBlood;
					break;
				case Flash_Vulnerable:
					ImpactEffectToPlay = ImpactEffectBlood;
					DamageToApply *= DamageMultiplier;
					break;
				case Concrete:
					ImpactEffectToPlay = ImpactEffectConcrete;
					break;
				default:
					ImpactEffectToPlay = ImpactEffectBlood;
					break;
			}

			UGameplayStatics::ApplyPointDamage(victim, DamageToApply, EyeRotation.Vector(), HitResult, MyOwner->GetInstigatorController(), this, damageType);			
			
			TrailEnd = HitResult.ImpactPoint; //gotta read smoke trail end
			if (ImpactEffectToPlay) {
				//what happens when it hits. Different effects go here
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffectToPlay, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
			}
		}

		FVector MuzzlePosition = MeshComp->GetSocketLocation(MuzzleSocket);

		if (MuzzleEffect) {
			//fire from muzzle. Not 100% sure on why this is "slightly" different from his...
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

		APlayerController* PlayerController = Cast<APlayerController>(MyOwner->GetController());
		if (PlayerController) {
			PlayerController->ClientPlayCameraShake(FireCameraShake);
		}

		UGameplayStatics::SpawnSoundAtLocation(this, ShotSound, MuzzlePosition);
	}
}

void ATPSWeapon::StartFire() {

	GetWorldTimerManager().SetTimer(BulletTimer, this, &ATPSWeapon::Fire, 0.5f, true, 0.0f);
}

void ATPSWeapon::EndFire() {

	GetWorldTimerManager().ClearTimer(BulletTimer);
}
