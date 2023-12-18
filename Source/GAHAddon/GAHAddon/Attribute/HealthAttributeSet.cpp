// Copyright (C) 2023 owoDra

#include "HealthAttributeSet.h"

#include "GameplayTag/GAHATags_Flag.h"
#include "GameplayTag/GAHATags_Damage.h"

#include "GameplayEffectExtension.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HealthAttributeSet)


void UHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, MinHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, ExtraHealth, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, MaxShield, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, DamageResistance, COND_None, REPNOTIFY_Always);
}


bool UHealthAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	/**
	 * Attribute [Damage]
	 */
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// Skip if damage amount is less than 0

		if (Data.EvaluatedData.Magnitude > 0.0f)
		{
			// If the damage is not self-destructive damage and during damage immunity, the amount of damage is reduced to 0.

			const auto bIsDamageFromSelfDestruct{ Data.EffectSpec.GetDynamicAssetTags().HasTagExact(TAG_Damage_Type_SelfDestruct) };
			const auto bHasDamageImmunity{ Data.Target.HasMatchingGameplayTag(TAG_Flag_DamageImmunity) };

			if (bHasDamageImmunity && !bIsDamageFromSelfDestruct)
			{
				Data.EvaluatedData.Magnitude = 0.0f;
				return false;
			}

			// Apply damage reduction due to damage resistance

			Data.EvaluatedData.Magnitude *= (1.0f - GetDamageResistance());
		}
	}

	return true;
}

void UHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	/**
	 * Attribute [Damage]
	 * 
	 * Apply Damage values to Health and Shield
	 */
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		auto DamageRemaing{ Data.EvaluatedData.Magnitude };

		// Skip if damage amount is less than 0

		if (DamageRemaing > 0.0f)
		{
			//FBEVerbMessage Message;
			//Message.Verb = TAG_Message_Damage;
			//Message.Instigator = Data.EffectSpec.GetEffectContext().GetEffectCauser();
			//Message.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
			//Message.Target = GetOwningActor();
			//Message.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
			////@TODO: Fill out context tags, and any non-ability-system source/instigator tags
			////@TODO: Determine if it's an opposing team kill, self-own, team kill, etc...
			//Message.Magnitude = Data.EvaluatedData.Magnitude;

			//UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			//MessageSystem.BroadcastMessage(Message.Verb, Message);
			
			while (DamageRemaing > 0)
			{
				auto DamageBuffer{ 0.0f };

				// Apply to ExtraHealth

				DamageBuffer = GetExtraHealth();
				if (DamageBuffer > 0)
				{
					SetExtraHealth(FMath::Max(0.0f, DamageBuffer - DamageRemaing));

					DamageRemaing -= DamageBuffer;

					continue;
				}

				// Apply to Shield

				DamageBuffer = GetShield();
				if (DamageBuffer > 0)
				{
					SetShield(FMath::Max(0.0f, DamageBuffer - DamageRemaing));

					DamageRemaing -= DamageBuffer;

					continue;
				}

				// Apply to Health

				DamageBuffer = GetHealth();
				if (DamageBuffer > 0)
				{
					SetHealth(FMath::Max(GetMinHealth(), DamageBuffer - DamageRemaing));

					DamageRemaing -= DamageBuffer;

					break;
				}

				break;
			}
		}

		SetDamage(0.0f);
	}

	
	/**
	 * Attribute [Healing]
	 *
	 * Apply Healing values to Health and Shield
	 */
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		auto HealRemaing{ Data.EvaluatedData.Magnitude };

		// Skip if healing amount is less than 0

		if (HealRemaing > 0.0f)
		{
			while (HealRemaing > 0)
			{
				auto HealingBuffer{ 0.0f };

				// Apply to Health

				HealingBuffer = GetMaxHealth() - GetHealth();
				if (HealingBuffer > 0)
				{
					SetHealth(FMath::Min(GetMaxHealth(), GetHealth() + HealRemaing));

					HealRemaing -= HealingBuffer;

					continue;
				}

				// Apply to Shield

				HealingBuffer = GetMaxShield() - GetShield();
				if (HealingBuffer > 0)
				{
					SetShield(FMath::Min(GetMaxShield(), GetShield() + HealRemaing));

					HealRemaing -= HealingBuffer;

					break;
				}

				break;
			}
		}

		SetHealing(0.0f);
	}

	/**
	 * Attribute [HealingShield]
	 *
	 * Apply HealingShield values to Shield
	 */
	else if (Data.EvaluatedData.Attribute == GetHealingShieldAttribute())
	{
		auto HealRemaing{ Data.EvaluatedData.Magnitude };

		// Skip if healing amount is less than 0

		if (HealRemaing > 0)
		{
			SetShield(FMath::Min(GetMaxShield(), GetShield() + HealRemaing));
		}

		SetHealingShield(0.0f);
	}

	/**
	 * Attribute [Health]
	 *
	 * Clamp Health Value
	 */
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), GetMinHealth(), GetMaxHealth()));
	}

	/**
	 * Attribute [Shield]
	 *
	 * Clamp Shield Value
	 */
	else if (Data.EvaluatedData.Attribute == GetShieldAttribute())
	{
		SetShield(FMath::Clamp(GetShield(), 0.0f, GetMaxShield()));
	}

	//  If Health is less than 0.0, Death is indicated.

	const auto bIsZeroHealth{ GetHealth() <= 0.0f };

	if (bIsZeroHealth && !bOutOfHealth)
	{
		if (OnOutOfHealth.IsBound())
		{
			const auto& EffectContext{ Data.EffectSpec.GetEffectContext() };
			auto* Instigator{ EffectContext.GetOriginalInstigator() };
			auto* Causer{ EffectContext.GetEffectCauser() };

			OnOutOfHealth.Broadcast(Instigator, Causer, Data.EffectSpec, Data.EvaluatedData.Magnitude);
		}
	}

	bOutOfHealth = bIsZeroHealth;
}

void UHealthAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UHealthAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	/**
	 * Attribute [MaxHealth]
	 *
	 * Clamp Health Value
	 */
	if (Attribute == GetMaxHealthAttribute())
	{
		if (GetHealth() > GetMaxHealth())
		{
			auto* ASC{ GetAbilitySystemComponent<UAbilitySystemComponent>() };
			check(ASC);

			ASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, GetMaxHealth());
		}
	}

	/**
	 * Attribute [MaxShield]
	 *
	 * Clamp Shield Value
	 */
	if (Attribute == GetMaxShieldAttribute())
	{
		if (GetShield() > GetMaxShield())
		{
			auto* ASC{ GetAbilitySystemComponent<UAbilitySystemComponent>() };
			check(ASC);

			ASC->ApplyModToAttribute(GetShieldAttribute(), EGameplayModOp::Override, GetMaxShield());
		}
	}

	// If Health exceeds 0 after application, remove flag

	if (bOutOfHealth && (GetHealth() > 0.0f))
	{
		bOutOfHealth = false;
	}
}

void UHealthAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	/**
	 * Attribute [Health]
	 *
	 * Clamp Health Value
	 */
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, GetMinHealth(), GetMaxHealth());
	}

	/**
	 * Attribute [MaxHealth]
	 *
	 * Clamp MaxHealth Value
	 */
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}

	/**
	 * Attribute [MinHealth]
	 *
	 * Clamp MinHealth Value
	 */
	else if (Attribute == GetMinHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}

	/**
	 * Attribute [ExtraHealth]
	 *
	 * Clamp ExtraHealth Value
	 */
	else if (Attribute == GetExtraHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}

	/**
	 * Attribute [Shield]
	 *
	 * Clamp Shield Value
	 */
	else if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
	}
	
	/**
	 * Attribute [MaxShield]
	 *
	 * Clamp MaxShield Value
	 */
	else if (Attribute == GetMaxShieldAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}


void UHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, Health, OldValue);
}

void UHealthAttributeSet::OnRep_MinHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, MinHealth, OldValue);
}

void UHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, MaxHealth, OldValue);
}

void UHealthAttributeSet::OnRep_ExtraHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, ExtraHealth, OldValue);
}

void UHealthAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, Shield, OldValue);
}

void UHealthAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, MaxShield, OldValue);
}

void UHealthAttributeSet::OnRep_DamageResistance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, DamageResistance, OldValue);
}
