// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
//generated has to be the last include in a file
#include "Tracker.generated.h"

class UNavigationSystemV1;
class USphereComponent;
class USoundCue;
class UHealthComponent;

UCLASS()
class LAB1_API ATracker : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATracker();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tracker Appearance Properties")
	UStaticMeshComponent* MeshComp;

	//Materials:
	UMaterialInstanceDynamic* MatInstance;

	UNavigationSystemV1* navSystem;
	FVector NextPoint;
	FVector GetNextPoint();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tracker Movement Properties")
	float MoveForce;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tracker Movement Properties")
	bool bUseVelocityChange;

	//Health & timer Parameters
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tracker Health Properties")
	UHealthComponent* HealthComp;
	bool bDestroyed;//default false
	bool bSelfDamageStarted; //default false
	UFUNCTION()
	void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float DeltaHealth, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	void DamageSelf();
	void SelfDestruct();

	//Explosion properties:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tracker Explosion Properties")
	UParticleSystem* ExplosionEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tracker Explosion Properties")
	USoundCue* ExplosionSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tracker Explosion Properties")
	float ExplosionRadius; //default to 500(uu)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tracker Explosion Properties")
	float ExplosionBaseDamage; //default to 100, but I halved it
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tracker Explosion Properties")
	USphereComponent* SelfDamageTrigger;
	FTimerHandle SelfDamageTimer;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
