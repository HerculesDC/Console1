// Fill out your copyright notice in the Description page of Project Settings.


#include "Tracker.h"
//Addition
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "TPSCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Sets default values
ATracker::ATracker()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	RootComponent = MeshComp;
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCanEverAffectNavigation(false);
	MoveForce = 1000.0f;
	bUseVelocityChange = true;
}

// Called when the game starts or when spawned
void ATracker::BeginPlay()
{
	Super::BeginPlay();
	
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
			//ALTERNATIVE:				   FindPathToLocationSynchronously(this, GetActorLocation(), player->GetActorLocation());
		if (path->PathPoints.Num() > 1) {
			DrawDebugSphere(GetWorld(), path->PathPoints[1], 30.0f, 12, FColor::Magenta, false, 1.0f, 0, 3.0f);
			return path->PathPoints[1];
		}
	}

	return GetActorLocation();
}

