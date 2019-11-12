// Fill out your copyright notice in the Description page of Project Settings.


#include "Grenade.h"
//Additions
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h" //apparently, this is required for GameplayStatics as well...
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
AGrenade::AGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	RootComponent = MeshComp;
	MeshComp->SetSimulatePhysics(true);
}

// Called when the game starts or when spawned
void AGrenade::BeginPlay()
{
	Super::BeginPlay();
	//the first float indicates loop timing. The one after the boolean introduces delay
	GetWorldTimerManager().SetTimer(expTimer, this, &AGrenade::Explode, 0.001f, false, expDelay);
}

// Called every frame
void AGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGrenade::Explode() {

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), expEffect, GetActorLocation());
	UGameplayStatics::SpawnSoundAtLocation(this, expSound, GetActorLocation());
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	//you've seen this before, but a refresher: nullptr is damage type, the second this is damage instigator, and the true at the end is "do full damage"
	//	there's another parameter after the bool, for damage prevention collision channel
	UGameplayStatics::ApplyRadialDamage(this, expBaseDamage, GetActorLocation(), expRadius, nullptr, IgnoreActors, this, GetInstigatorController(), true);
	Destroy();
}