// Copyright (C) 2023 owoDra

#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "DamageExecution.generated.h"


/**
 * Exection of Gameplay Effect to inflict damage
 */
UCLASS()
class GAHADDON_API UDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	UDamageExecution();

protected:
	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	/**
	 * Exection of Gameplay Effect to inflict damage
	 */
	virtual float ModifiyDamage(float BaseDamage, const FGameplayEffectCustomExecutionParameters& ExecutionParams) const { return BaseDamage; }

};
