// Copyright (C) 2023 owoDra

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
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

};
