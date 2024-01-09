// Copyright (C) 2024 owoDra

#pragma once

#include "GAEGameplayAbility.h"

#include "GameplayAbility_Death.generated.h"

class UHealthComponent;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;


/**
 * Gameplay ability used for handling death.
 * Ability is activated automatically via the "Event.OutOfHealth" ability trigger tag.
 */
UCLASS(Abstract)
class UGameplayAbility_Death : public UGAEGameplayAbility
{
	GENERATED_BODY()
public:
	UGameplayAbility_Death(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//
	// If enabled, the ability will automatically call StartDeath when activate. 
	// 
	// Tips:
	//	FinishDeath is always called when the ability ends if the death was started.
	// 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	bool bAutoStartDeath{ true };

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	//
	// Starts the death sequence.
	//
	UFUNCTION(BlueprintCallable, Category = "Death")
	void StartDeath();

	//
	// Finishes the death sequence.
	//
	UFUNCTION(BlueprintCallable, Category = "Death")
	void FinishDeath();

protected:
	UHealthComponent* GetHealthComponent() const;

};
