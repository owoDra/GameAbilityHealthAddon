// Copyright (C) 2024 owoDra

#include "HealExecution.h"

#include "Attribute/HealthAttributeSet.h"
#include "Attribute/CombatAttributeSet.h"
#include "HealthExecutionModifier.h"

#include "GameplayEffectTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HealExecution)


#pragma region HealStatics

struct FHealStatics
{
	FGameplayEffectAttributeCaptureDefinition BaseHealDef;

	FHealStatics()
	{
		BaseHealDef = FGameplayEffectAttributeCaptureDefinition(
			UCombatAttributeSet::GetBaseHealAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static FHealStatics& HealStatics()
{
	static FHealStatics Statics;
	return Statics;
}

#pragma endregion


#pragma region HealShieldStatics

struct FHealShieldStatics
{
	FGameplayEffectAttributeCaptureDefinition BaseHealDef;

	FHealShieldStatics()
	{
		BaseHealDef = FGameplayEffectAttributeCaptureDefinition(
			UCombatAttributeSet::GetBaseHealShieldAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static FHealShieldStatics& HealShieldStatics()
{
	static FHealShieldStatics Statics;
	return Statics;
}

#pragma endregion


#pragma region HealExecution

UHealExecution::UHealExecution()
{
	RelevantAttributesToCapture.Add(HealStatics().BaseHealDef);
}

void UHealExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	auto Heal{ 0.0f };
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HealStatics().BaseHealDef, EvaluateParameters, Heal);

	for (const auto& Modifier : Modifiers)
	{
		if (Modifier)
		{
			Heal = Modifier->ModifierExecution(Heal, ExecutionParams);
		}
	}

	Heal = FMath::Max(0.0f, Heal);

	if (Heal > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UHealthAttributeSet::GetHealingAttribute(), EGameplayModOp::Additive, Heal));
	}

#endif // #if WITH_SERVER_CODE
}

#pragma endregion


#pragma region HealShieldExecution

UHealShieldExecution::UHealShieldExecution()
{
	RelevantAttributesToCapture.Add(HealShieldStatics().BaseHealDef);
}

void UHealShieldExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	auto Heal{ 0.0f };
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HealShieldStatics().BaseHealDef, EvaluateParameters, Heal);

	for (const auto& Modifier : Modifiers)
	{
		if (Modifier)
		{
			Heal = Modifier->ModifierExecution(Heal, ExecutionParams);
		}
	}

	Heal = FMath::Max(0.0f, Heal);

	if (Heal > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UHealthAttributeSet::GetHealingShieldAttribute(), EGameplayModOp::Additive, Heal));
	}

#endif // #if WITH_SERVER_CODE
}

#pragma endregion
