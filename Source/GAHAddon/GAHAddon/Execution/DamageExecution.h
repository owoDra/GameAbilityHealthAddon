// Copyright (C) 2024 owoDra

#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "DamageExecution.generated.h"

class UHealthExecutionModifier;


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
	//
	// List of additional calculation processes
	//
	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<TObjectPtr<UHealthExecutionModifier>> Modifiers;

protected:
	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

};
