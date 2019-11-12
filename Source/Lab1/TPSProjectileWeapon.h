// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TPSWeapon.h"
#include "TPSProjectileWeapon.generated.h"

/**
 * 
 */

class AGrenade;
UCLASS()
class LAB1_API ATPSProjectileWeapon : public ATPSWeapon
{
	GENERATED_BODY()

public:
	ATPSProjectileWeapon();

protected:
	
	void BeginPlay() override;

	//UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire();//not sure whether to call override here

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammo")
	TSubclassOf<AGrenade> Projectile;
	AGrenade* Grenade;

	UPROPERTY(EditDefaultsOnly, Category = "Launch Force")
	float LaunchForce;

private:
	void Tick(float deltaTime) override;
};
