// Copyright (C) 2024 owoDra

#pragma once

#include "HealthExecutionModifier.generated.h"

struct FGameplayEffectCustomExecutionParameters;


/**
 * Base class for additional computational processing that can be used in HealExecution or DamageExecution
 */
UCLASS(BlueprintType, Const, DefaultToInstanced, EditInlineNew)
class GAHADDON_API UHealthExecutionModifier : public UObject
{
	GENERATED_BODY()
public:
	UHealthExecutionModifier(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	/**
	 * Base class that defines and executes the method of automatic team assignment
	 */
	virtual float ModifierExecution(float Base, const FGameplayEffectCustomExecutionParameters& ExecutionParams) const { return Base; }

};
