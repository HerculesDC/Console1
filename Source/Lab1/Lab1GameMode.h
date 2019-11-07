// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Lab1GameMode.generated.h"

/**
 * 
 */
UCLASS()
class LAB1_API ALab1GameMode : public AGameModeBase
{
	GENERATED_BODY()

	public:

		ALab1GameMode();

	protected:

		FTimerHandle TimerHandle_BotSpawner;
		FTimerHandle TimerHandle_WaveManager;
		UPROPERTY(EditDefaultsOnly, Category = "Game Mode")
		float TimeBetweenWaves;
		UPROPERTY(EditDefaultsOnly, Category = "Game Mode")
		float spawnRate;
		UPROPERTY(EditDefaultsOnly, Category = "Game Mode")
		int32 numberOfBotsToSpawn;
		int32 waveNumber;
		//function to spawn bots
		UFUNCTION(BlueprintImplementableEvent, Category = "Game Mode")
		void SpawnNewBot(); //DO NOT IMPLEMENT IN C++

		//internal functions
		void SpawnBotTimerElapsed();
		void StartWave();
		void EndWave();
		void CheckWaveState();
		void PrepareForNextWave();

	public:
		virtual void StartPlay() override;
		virtual void Tick(float deltaTime) override;
};
