// Fill out your copyright notice in the Description page of Project Settings.

#include "TPSCharacter.h"
//Additions
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "TPSWeapon.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "HealthComponent.h"
#include "Lab1.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATPSCharacter::ATPSCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//ADDITION: "Unreal Way" of creating objects (templated factory methods)
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm Component"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
	CameraComp->SetupAttachment(SpringArmComp);

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ATPSCharacter::OnHealthChanged);
}

// Called when the game starts or when spawned
void ATPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	EndZoom();

	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentWeapon = GetWorld()->SpawnActor<ATPSWeapon>(StarterWeaponClass, spawnParams);
	if (CurrentWeapon) {
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(Cast<USceneComponent>(GetMesh()), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
	}
}

void ATPSCharacter::OnHealthChanged(UHealthComponent * OwingHealthComp, float Health, float DeltaHealth, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser) {

	if (Health <= 0) {
		bDead = true;

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DetachFromControllerPendingDestroy();
		SetLifeSpan(5.0f);
		GetWorldTimerManager().SetTimer(WeaponDetachTimer, this, &ATPSCharacter::DetachWeapon, 3.0f, false, 0.2f);
		GetMesh()->CreateAndSetMaterialInstanceDynamicFromMaterial(0, deathMaterial);
		GetMesh()->SetScalarParameterValueOnMaterials("StartTime", UGameplayStatics::GetRealTimeSeconds(this));
	}
}

void ATPSCharacter::DetachWeapon() {
	CurrentWeapon->MeshComp->SetSimulatePhysics(true);
	CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
}

// Called every frame
void ATPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	dt = DeltaTime;

	//NOTE: FRotator(pitch, yaw, roll) refers to X, Z and Y rotations (because Unreal's Z is up);
	if (bInCover) {
		if (bIsAiming) {
			GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
		}
		else {
			if (OverlappingBoxCollision) {
				GetMesh()->SetWorldRotation(OverlappingBoxCollision->GetComponentRotation().Add(0.0f, 40.0f, 0.0f));
			}
		}
	}
	else {
		GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	}
}

//MoveForward Definition:
void ATPSCharacter::MoveForward(float Val) {

	if(!bInCover) GetCharacterMovement()->AddInputVector(GetActorForwardVector()*Val);
}

void ATPSCharacter::MoveRight(float Val) {

	if (bInCover) {
		
		FVector targetPos = GetActorLocation() + (OverlappingBoxCollision->GetRightVector()*Val*dt);
		FVector offsetFromCoverCenter = targetPos - OverlappingBoxCollision->GetComponentLocation();
		
		float dot = FVector::DotProduct(offsetFromCoverCenter, OverlappingBoxCollision->GetRightVector());
		
		bool isTargetInRange = abs(dot) < (OverlappingBoxCollision->GetScaledBoxExtent().Y-50);
		bool isMovingTowardsCentre = dot * Val < 0;
		
		if (isTargetInRange || isMovingTowardsCentre) {
		GetCharacterMovement()->AddInputVector(OverlappingBoxCollision->GetRightVector()*Val);
		}
	}
	else GetCharacterMovement()->AddInputVector(GetActorRightVector()*Val);
}

void ATPSCharacter::BeginCrouch() {
	Crouch();
}

void ATPSCharacter::EndCrouch() {
	UnCrouch();
}

void ATPSCharacter::Jump() {
	ACharacter::Jump();
}

void ATPSCharacter::TakeCover() {

	if (OverlappingBoxCollision) {

		if (bInCover) { 
			bInCover = false;
		}
		else {
			FVector lineTraceStart = GetActorLocation();
			FVector lineTraceEnd = GetActorLocation() + OverlappingBoxCollision->GetForwardVector() * 10000;

			FHitResult hit;
			//CHANGE THIS TO COVERTRACECHANNEL
			if (GetWorld()->LineTraceSingleByChannel(hit, lineTraceStart, lineTraceEnd, CoverTraceChannel)) {
				FVector targetLocation = hit.Location;
				targetLocation -= OverlappingBoxCollision->GetForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
				SetActorLocation(targetLocation);
				bInCover = true;
			}
		}
	}
}

// Called to bind functionality to input
void ATPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//function binding
	//CAREFUL: SOME FUNCTIONS ARE PARENT-CLASS-BASED
	PlayerInputComponent->BindAxis("MoveForward", this, &ATPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATPSCharacter::MoveRight);
	//pitch: up-down, Yaw: Left-right, Roll: rotation along forward axis(leaning)
	PlayerInputComponent->BindAxis("LookUp", this, &ACharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookSideWays", this, &ACharacter::AddControllerYawInput);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ATPSCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("UnCrouch", IE_Released, this, &ATPSCharacter::EndCrouch);

	PlayerInputComponent->BindAction("TakeCover", IE_Pressed, this, &ATPSCharacter::TakeCover);
	
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ATPSCharacter::StartZoom);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ATPSCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATPSCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ATPSCharacter::EndFire);
}

FVector ATPSCharacter::GetPawnViewLocation() const {

	if (CameraComp) return CameraComp->GetComponentLocation();

	return Super::GetPawnViewLocation();
}

void ATPSCharacter::StartZoom() {
	bIsAiming = true;
	CameraComp->SetFieldOfView(zoomedFoV);
}

void ATPSCharacter::EndZoom() {
	bIsAiming = false;
	CameraComp->SetFieldOfView(defaultFoV);
}

void ATPSCharacter::FireWeapon() {

	if (CurrentWeapon) {
		CurrentWeapon->Fire();
	}
}

void ATPSCharacter::StartFire() {

	if (CurrentWeapon) {
		CurrentWeapon->StartFire();
	}
}

void ATPSCharacter::EndFire() {
	if (CurrentWeapon) {
		CurrentWeapon->EndFire();
	}
}
