// Fill out your copyright notice in the Description page of Project Settings.


#include "Tracker.h"
//Addition
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "TPSCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "HealthComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

// Sets default values
ATracker::ATracker()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	RootComponent = MeshComp;
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCanEverAffectNavigation(false);
	MoveForce = 100.0f;
	bUseVelocityChange = true;

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ATracker::OnHealthChanged);
	bDestroyed = false;
	bSelfDamageStarted = false;

	SelfDamageTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("Self Damage Trigger"));
	SelfDamageTrigger->SetSphereRadius(250.0f);
	SelfDamageTrigger->SetupAttachment(RootComponent);
	SelfDamageTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SelfDamageTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SelfDamageTrigger->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	ExplosionRadius = 500.0f;//uu, or cm
	ExplosionBaseDamage = 50.0f;
}

// Called when the game starts or when spawned
void ATracker::BeginPlay()
{
	Super::BeginPlay();
	navSystem = UNavigationSystemV1::GetCurrent(this);
	NextPoint = GetNextPoint();
}

// Called every frame
void ATracker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float distanceToTarget = (NextPoint - GetActorLocation()).Size();
	
	if (distanceToTarget < 100.0f) {
		NextPoint = GetNextPoint();
	}
	else {
		FVector force = NextPoint - GetActorLocation();
		force.Normalize();
		force *= MoveForce;
		MeshComp->AddForce(force, NAME_None, bUseVelocityChange);
		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + force, 20.0f, FColor::Green, false, 2.0f*DeltaTime, 0, 3.0f);
	}
}

// Called to bind functionality to input
void ATracker::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

FVector ATracker::GetNextPoint() {

	ATPSCharacter* player = Cast<ATPSCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (player) {
		
		UNavigationPath* path = navSystem->FindPathToActorSynchronously(this, GetActorLocation(), player);
							//ALTERNATIVE: FindPathToLocationSynchronously(this, GetActorLocation(), player->GetActorLocation());

		if (path->PathPoints.Num() > 1) {
			DrawDebugSphere(GetWorld(), path->PathPoints[1], 30.0f, 12, FColor::Magenta, false, 1.0f, 0, 3.0f);
			return path->PathPoints[1];
		}
	}
	return GetActorLocation();
}

void ATracker::SelfDestruct() {
	
	if (!bDestroyed) {
		bDestroyed = true;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
		UGameplayStatics::SpawnSoundAtLocation(this,ExplosionSound, GetActorLocation());
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		UGameplayStatics::ApplyRadialDamage(this, ExplosionBaseDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);
		//24 refers to sections, 5.0f refers to time, 0 refers to masks, 2.0 refers to thickness
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 24, FColor::Orange, false, 5.0f, 0, 2.0f);
		Destroy();
	}
}

void ATracker::OnHealthChanged(UHealthComponent* OwingHealthComp, float Health, float DeltaHealth, const class UDamageType* DamageType, class AController* InstrigatedBy, AActor* DamageCauser) {

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, "Tracker Bot Damaged: " + FString::SanitizeFloat(Health));

	if (!MatInstance) {
		MatInstance = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MatInstance) {
		MatInstance->SetScalarParameterValue("LastTimeHit", GetWorld()->TimeSeconds);
	}
	if (Health <= 0) {
		SelfDestruct();
	}
}

void ATracker::DamageSelf() {
	//apply 20 damage to self. nullptr refers to damage type
	UGameplayStatics::ApplyDamage(this, 20.0f, GetInstigatorController(), this, nullptr);
}

void ATracker::NotifyActorBeginOverlap(AActor* OtherActor) {

	if (!bSelfDamageStarted) {

		ATPSCharacter* player = Cast<ATPSCharacter>(OtherActor);

		if (player) {//sets timer to start and loop the "damage self" function
			//consider changing the self-damage ratio (0.5f) and storing it in a UPROPERTY variable
			GetWorldTimerManager().SetTimer(SelfDamageTimer, this, &ATracker::DamageSelf, 0.5f, true, 0.0f);
			MoveForce = 0.0f;
			bSelfDamageStarted = true;
		}
	}
}