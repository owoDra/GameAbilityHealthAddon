// Copyright (C) 2023 owoDra

#include "DamageExecution.h"

#include "Attribute/HealthAttributeSet.h"
#include "Attribute/CombatAttributeSet.h"
#include "HealthExecutionModifier.h"

#include "GameplayEffectTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DamageExecution)


#pragma region DamageStatics

struct FDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition BaseDamageDef;

	FDamageStatics()
	{
		BaseDamageDef = FGameplayEffectAttributeCaptureDefinition(
			UCombatAttributeSet::GetBaseDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static FDamageStatics& DamageStatics()
{
	static FDamageStatics Statics;
	return Statics;
}

#pragma endregion


#pragma region DamageExecution

UDamageExecution::UDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().BaseDamageDef);
}

void UDamageExecution::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE

	const auto& Spec{ ExecutionParams.GetOwningSpec() };
	const auto* SourceTags{ Spec.CapturedSourceTags.GetAggregatedTags() };
	const auto* TargetTags{ Spec.CapturedTargetTags.GetAggregatedTags() };

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	auto Damage{ 0.0f };
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BaseDamageDef, EvaluateParameters, Damage);

	for (const auto& Modifier : Modifiers)
	{
		if (Modifier)
		{
			Damage = Modifier->ModifierExecution(Damage, ExecutionParams);
		}
	}

	if (Damage > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UHealthAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, Damage));
	}

#endif // #if WITH_SERVER_CODE
}

#pragma endregion
