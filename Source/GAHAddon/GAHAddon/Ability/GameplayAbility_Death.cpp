// Copyright (C) 2023 owoDra

#include "GameplayAbility_Death.h"

#include "GameplayTag/GAHATags_Event.h"
#include "GameplayTag/GAHATags_Ability.h"
#include "HealthFunctionLibrary.h"
#include "HealthComponent.h"
#include "GAHAddonLogs.h"

#include "GameplayTag/GAETags_Ability.h"

#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayAbility_Death)


UGameplayAbility_Death::UGameplayAbility_Death(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	ActivationMethod = EAbilityActivationMethod::Custom;

	AbilityTags.AddTag(TAG_Ability_Type_Misc);

	// Add the ability trigger tag as default to the CDO.

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = TAG_Event_OutOfHealth;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

		AbilityTriggers.Add(TriggerData);
	}
}


void UGameplayAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);

	auto* ASC{ ActorInfo->AbilitySystemComponent.Get() };

	// Make it non-cancelable

	SetCanBeCanceled(false);

	// Cancel all abilities and block others from starting.

	FGameplayTagContainer AbilityTypesToIgnore;
	AbilityTypesToIgnore.AddTag(TAG_Ability_Behavior_ActiveIgnoreDeath);

	ASC->CancelAbilities(nullptr, &AbilityTypesToIgnore, this);

	// Execute DeathStart if possible

	if (bAutoStartDeath)
	{
		StartDeath();
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	check(ActorInfo);

	// Always try to finish the death when the ability ends in case the ability doesn't.
	// This won't do anything if the death hasn't been started.

	FinishDeath();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


void UGameplayAbility_Death::StartDeath()
{
	if (auto* HealthComponent{ GetHealthComponent() })
	{
		if (HealthComponent->GetDeathState() == EDeathState::NotDead)
		{
			HealthComponent->HandleStartDeath();
		}
	}
}

void UGameplayAbility_Death::FinishDeath()
{
	if (auto* HealthComponent{ GetHealthComponent() })
	{
		if (HealthComponent->GetDeathState() == EDeathState::DeathStarted)
		{
			HealthComponent->HandleFinishDeath();
		}
	}
}


UHealthComponent* UGameplayAbility_Death::GetHealthComponent() const
{
	if (auto* HCFromSourceObject{ GetTypedSourceObject<UHealthComponent>() })
	{
		return HCFromSourceObject;
	}
	else if (auto* HCFromAvatar{ UHealthFunctionLibrary::GetHealthComponentFromActor(GetAvatarActorFromActorInfo()) })
	{
		UE_LOG(LogGAHA, Warning, TEXT("UGameplayAbility_Death::GetHealthComponent: HealthComponent could not be get from SourceObject, so it was get from AvatarActor."));

		return HCFromAvatar;
	}

	UE_LOG(LogGAHA, Error, TEXT("UGameplayAbility_Death::GetHealthComponent: Could not get HealthComponent."));

	return nullptr;
}
