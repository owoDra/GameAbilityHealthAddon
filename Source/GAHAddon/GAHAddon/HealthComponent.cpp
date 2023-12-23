// Copyright (C) 2023 owoDra

#include "HealthComponent.h"

#include "Attribute/HealthAttributeSet.h"
#include "Attribute/CombatAttributeSet.h"
#include "HealthData.h"
#include "GameplayTag/GAHATags_Status.h"
#include "GAHAddonLogs.h"

#include "GAEAbilitySystemComponent.h"

#include "InitState/InitStateTags.h"

#include "AbilitySystemGlobals.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HealthComponent)


static AActor* GetInstigatorFromAttrChangeData(const FOnAttributeChangeData& ChangeData)
{
	if (ChangeData.GEModData != nullptr)
	{
		const auto& EffectContext{ ChangeData.GEModData->EffectSpec.GetEffectContext() };
		return EffectContext.GetOriginalInstigator();
	}

	return nullptr;
}


const FName UHealthComponent::NAME_ActorFeatureName("Health");

UHealthComponent::UHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, DeathState);
	DOREPLIFETIME(UHealthComponent, HealthData);
}


void UHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeFromAbilitySystem();

	Super::EndPlay(EndPlayReason);
}


void UHealthComponent::InitializeWithAbilitySystem()
{
	auto* Owner{ GetOwner() };
	check(Owner);

	auto* NewASC{ UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner) };

	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent == NewASC)
		{
			return;
		}

		UninitializeFromAbilitySystem();
	}

	AbilitySystemComponent = NewASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogGAHA, Error, TEXT("HealthComponent: Cannot initialize health component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	HealthSet = Cast<UHealthAttributeSet>(AbilitySystemComponent->InitStats(UHealthAttributeSet::StaticClass(), nullptr));
	if (!HealthSet)
	{
		UE_LOG(LogGAHA, Error, TEXT("HealthComponent: Cannot initialize health component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	CombatSet = Cast<UCombatAttributeSet>(AbilitySystemComponent->InitStats(UCombatAttributeSet::StaticClass(), nullptr));
	if (!CombatSet)
	{
		UE_LOG(LogGAHA, Error, TEXT("HealthComponent: Cannot initialize health component for owner [%s] with NULL combat set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	UE_LOG(LogGAHA, Log, TEXT("[%s] Health Component initialize with Ability System(%s), AttibuteSet(Health: %s, Combat: %s)"), 
		Owner->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"), *GetNameSafe(AbilitySystemComponent), *GetNameSafe(HealthSet), *GetNameSafe(CombatSet));

	// Bind Events

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetMinHealthAttribute()).AddUObject(this, &ThisClass::HandleMinHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetExtraHealthAttribute()).AddUObject(this, &ThisClass::HandleExtraHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetShieldAttribute()).AddUObject(this, &ThisClass::HandleShieldChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetMaxShieldAttribute()).AddUObject(this, &ThisClass::HandleMaxShieldChanged);
	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);
	HealthSet->OnDamaged.AddUObject(this, &ThisClass::HandleOnDamaged);
	HealthSet->OnHealed.AddUObject(this, &ThisClass::HandleOnHealed);

	ApplyHealthData();
}

void UHealthComponent::UninitializeFromAbilitySystem()
{
	ClearGameplayTags();

	if (HealthSet)
	{
		HealthSet->OnOutOfHealth.RemoveAll(this);
		HealthSet->OnDamaged.RemoveAll(this);
		HealthSet->OnHealed.RemoveAll(this);
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetHealthAttribute()).RemoveAll(this);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetMinHealthAttribute()).RemoveAll(this);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetExtraHealthAttribute()).RemoveAll(this);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetShieldAttribute()).RemoveAll(this);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetMaxShieldAttribute()).RemoveAll(this);

	}

	AbilitySystemComponent = nullptr;
	HealthSet = nullptr;
	CombatSet = nullptr;
}

void UHealthComponent::ClearGameplayTags()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Status_Death_Dying, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Status_Death_Dead, 0);
	}
}


void UHealthComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	Super::OnActorInitStateChanged(Params);

	// Wait for initialization of AbilitySystemCompoenet

	if (Params.FeatureName == UGAEAbilitySystemComponent::NAME_ActorFeatureName)
	{
		if ((Params.FeatureState == TAG_InitState_DataInitialized))
		{
			CheckDefaultInitialization();
		}
	}
}

bool UHealthComponent::CanChangeInitStateToDataInitialized(UGameFrameworkComponentManager* Manager) const
{
	if (!HealthData)
	{
		return false;
	}

	if (!Manager->HasFeatureReachedInitState(GetOwner(), UGAEAbilitySystemComponent::NAME_ActorFeatureName, TAG_InitState_DataInitialized))
	{
		return false;
	}

	return true;
}

void UHealthComponent::HandleChangeInitStateToDataInitialized(UGameFrameworkComponentManager* Manager)
{
	InitializeWithAbilitySystem();
}


void UHealthComponent::OnRep_HealthData()
{
	CheckDefaultInitialization();
}

void UHealthComponent::ApplyHealthData()
{
	check(HealthData);
	check(AbilitySystemComponent);

	auto* Owner{ GetOwner() };
	check(Owner);

	UE_LOG(LogGAHA, Log, TEXT("[%s] Health Component applies Health Data(%s)"),
		Owner->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"), *GetNameSafe(HealthData));

	// Set the initial value of Attribute

	AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetMaxHealthAttribute(), HealthData->MaxHealth);
	AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetMinHealthAttribute(), HealthData->MinHealth);
	AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), HealthData->Health);

	AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetExtraHealthAttribute(), HealthData->ExtraHealth);

	AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetMaxShieldAttribute(), HealthData->MaxShield);
	AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetShieldAttribute(), HealthData->Shield);

	ClearGameplayTags();

	// Broadcast delegates

	OnHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
	OnMaxHealthChanged.Broadcast(this, HealthSet->GetMaxHealth(), HealthSet->GetMaxHealth(), nullptr);
	OnMinHealthChanged.Broadcast(this, HealthSet->GetMinHealth(), HealthSet->GetMinHealth(), nullptr);

	OnExtraHealthChanged.Broadcast(this, HealthSet->GetExtraHealth(), HealthSet->GetExtraHealth(), nullptr);

	OnShieldChanged.Broadcast(this, HealthSet->GetShield(), HealthSet->GetShield(), nullptr);
	OnMaxShieldChanged.Broadcast(this, HealthSet->GetMaxShield(), HealthSet->GetMaxShield(), nullptr);
}

void UHealthComponent::SetHealthData(const UHealthData* NewHealthData)
{
	if (GetOwner()->HasAuthority())
	{
		if (NewHealthData != HealthData)
		{
			HealthData = NewHealthData;

			CheckDefaultInitialization();
		}
	}
}


void UHealthComponent::OnRep_DeathState()
{
	switch (DeathState)
	{
	case EDeathState::DeathStarted:
		HandleStartDeath();
		break;

	case EDeathState::DeathFinished:
		HandleFinishDeath();
		break;
	}
}

void UHealthComponent::HandleStartDeath()
{
	auto* Owner{ GetOwner() };
	check(Owner);

	if (Owner->HasAuthority())
	{
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Status_Death_Dying, 1);
		}
	}

	OnDeathStarted.Broadcast(Owner);
}

void UHealthComponent::HandleFinishDeath()
{
	auto* Owner{ GetOwner() };
	check(Owner);

	if (Owner->HasAuthority())
	{
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Status_Death_Dead, 1);
		}
	}

	OnDeathFinished.Broadcast(Owner);
}

void UHealthComponent::StartDeath()
{
	auto* Owner{ GetOwner() };

	if (Owner->HasAuthority())
	{
		if (DeathState == EDeathState::NotDead)
		{
			DeathState = EDeathState::DeathStarted;

			HandleStartDeath();

			Owner->ForceNetUpdate();
		}
	}
}

void UHealthComponent::FinishDeath()
{
	auto* Owner{ GetOwner() };

	if (Owner->HasAuthority())
	{
		if (DeathState == EDeathState::DeathStarted)
		{
			DeathState = EDeathState::DeathFinished;

			HandleFinishDeath();

			Owner->ForceNetUpdate();
		}
	}
}


void UHealthComponent::HandleHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	OnHealthChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
}

void UHealthComponent::HandleMaxHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	OnMaxHealthChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
}

void UHealthComponent::HandleMinHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	OnMinHealthChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
}

void UHealthComponent::HandleExtraHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	OnExtraHealthChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
}

void UHealthComponent::HandleShieldChanged(const FOnAttributeChangeData& ChangeData)
{
	OnShieldChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
}

void UHealthComponent::HandleMaxShieldChanged(const FOnAttributeChangeData& ChangeData)
{
	OnMaxShieldChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
}

void UHealthComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{

}

void UHealthComponent::HandleOnDamaged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{

}

void UHealthComponent::HandleOnHealed(AActor* HealInstigator, AActor* HealCauser, const FGameplayEffectSpec& HealEffectSpec, float HealMagnitude)
{

}


float UHealthComponent::GetHealth() const
{
	return (HealthSet ? HealthSet->GetHealth() : 0.0f);
}

float UHealthComponent::GetMaxHealth() const
{
	return (HealthSet ? HealthSet->GetMaxHealth() : 0.0f);
}

float UHealthComponent::GetMinHealth() const
{
	return (HealthSet ? HealthSet->GetMinHealth() : 0.0f);
}

float UHealthComponent::GetExtraHealth() const
{
	return (HealthSet ? HealthSet->GetExtraHealth() : 0.0f);
}

float UHealthComponent::GetShield() const
{
	return (HealthSet ? HealthSet->GetShield() : 0.0f);
}

float UHealthComponent::GetMaxShield() const
{
	return (HealthSet ? HealthSet->GetMaxShield() : 0.0f);
}

float UHealthComponent::GetTotalHealth() const
{
	return (GetHealth() + GetShield() + GetExtraHealth());
}

float UHealthComponent::GetTotalMaxHealth() const
{
	return (GetMaxHealth() + GetMaxShield() + GetExtraHealth());
}


UHealthComponent* UHealthComponent::FindHealthComponent(const AActor* Actor)
{
	return (Actor ? Actor->FindComponentByClass<UHealthComponent>() : nullptr);
}
