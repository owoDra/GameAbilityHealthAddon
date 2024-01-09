// Copyright (C) 2024 owoDra

#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "HealExecution.generated.h"


/**
 * Gameplay Effect Exection for HP Healing
 */
UCLASS()
class GAHADDON_API UHealExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	UHealExecution();

protected:
	//
	// List of additional calculation processes
	//
	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<TObjectPtr<UHealthExecutionModifier>> Modifiers;

protected:
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

};


/**
 * Gameplay Effect Exection for Shield Healing
 */
UCLASS()
class GAHADDON_API UHealShieldExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	UHealShieldExecution();

protected:
	//
	// List of additional calculation processes
	//
	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<TObjectPtr<UHealthExecutionModifier>> Modifiers;

protected:
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

};
