// Copyright (C) 2024 owoDra

#pragma once

#include "GAEAttributeSet.h"

#include "AbilitySystemComponent.h"

#include "HealthAttributeSet.generated.h"


/**
 * Classes that define attributes such as character health, shields, max strength, max shields, etc.
 */
UCLASS(BlueprintType)
class UHealthAttributeSet : public UGAEAttributeSet
{
	GENERATED_BODY()
public:
	UHealthAttributeSet() {}

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	/**
	 * Clamp the value of certain attributes
	 */
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;


public:
	//
	// Delegate to broadcast when a heal is received
	//
	mutable FAttributeEvent OnOutOfHealth;

private:
	//
	// Used to track when the health reaches 0.
	//
	bool bOutOfHealth{ false };

public:
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, MinHealth);
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, MaxHealth);

	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, ExtraHealth);

	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Shield);
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, MaxShield);

	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, DamageResistance);

protected:
	UFUNCTION() void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MinHealth(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION() void OnRep_ExtraHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION() void OnRep_Shield(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MaxShield(const FGameplayAttributeData& OldValue);

	UFUNCTION() void OnRep_DamageResistance(const FGameplayAttributeData& OldValue);

private:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Health{ 100.0f };

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MinHealth, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MinHealth{ 0.0f };

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth{ 100.0f };


	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ExtraHealth, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData ExtraHealth{ 0.0f };


	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Shield, Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Shield{ 50.0f };

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxShield, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxShield{ 50.0f };


	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageResistance, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData DamageResistance{ 0.0f };


	/////////////////////////////////////////////////////////////////////
	//	Meta attributes (the following are not "stateful" attributes)
	/////////////////////////////////////////////////////////////////////
public:
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Healing);
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, HealingShield);
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Damage);

private:
	//
	// Amount of HP heal. This value directly increases the Health value.
	//
	UPROPERTY(BlueprintReadOnly, Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Healing{ 0.0f };

	//
	// Amount of Shield heal. This value directly increases the Shield value.
	//
	UPROPERTY(BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData HealingShield{ 0.0f };

	//
	// Amount of Damage. This value directly reduces the Health value.
	//
	UPROPERTY(BlueprintReadOnly, Meta=(HideFromModifiers, AllowPrivateAccess=true))
	FGameplayAttributeData Damage{ 0.0f };

};
