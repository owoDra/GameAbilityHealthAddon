// Copyright (C) 2024 owoDra

#pragma once

#include "GAEAttributeSet.h"

#include "AbilitySystemComponent.h"

#include "CombatAttributeSet.generated.h"


/**
 * Class that defines attributes for the amount of healing and damage from combat
 */
UCLASS(BlueprintType)
class UCombatAttributeSet : public UGAEAttributeSet
{
	GENERATED_BODY()
public:
	UCombatAttributeSet() {}

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, BaseDamage);
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, BaseHeal);
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, BaseHealShield);

protected:
	UFUNCTION() void OnRep_BaseDamage(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_BaseHeal(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_BaseHealShield(const FGameplayAttributeData& OldValue);

private:
	//
	// Amount of actual damage done
	//
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseDamage, Category = "Attribute|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseDamage{ 0.0f };

	//
	// Amount of actual heal done
	//
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseHeal, Category = "Attribute|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseHeal{ 0.0f };

	//
	// Amount of actual shield heal done
	//
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseHealShield, Category = "Attribute|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseHealShield{ 0.0f };

};
