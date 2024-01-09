// Copyright (C) 2024 owoDra

#include "HealthComponent.h"

#include "Attribute/HealthAttributeSet.h"
#include "Attribute/CombatAttributeSet.h"
#include "HealthData.h"
#include "Message/HealthMessageTypes.h"
#include "GameplayTag/GAHATags_Status.h"
#include "GameplayTag/GAHATags_Message.h"
#include "GameplayTag/GAHATags_Event.h"
#include "GAHAddonLogs.h"

#include "GAEAbilitySystemComponent.h"

#include "Message/GameplayMessageSubsystem.h"
#include "InitState/InitStateTags.h"

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemGlobals.h"

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

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_None;

	DOREPLIFETIME_WITH_PARAMS_FAST(UHealthComponent, HealthData, Params);
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
		if (DeathAbilitySpecHandle.IsValid())
		{
			AbilitySystemComponent->CancelAbilityHandle(DeathAbilitySpecHandle);
			AbilitySystemComponent->ClearAbility(DeathAbilitySpecHandle);
		}

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

	if (auto DeathAbilityClass{ HealthData->DeathEventAbilityClass })
	{
		auto* AbilityCDO{ DeathAbilityClass.GetDefaultObject() };
		auto AbilitySpec{ FGameplayAbilitySpec(AbilityCDO, 1) };
		AbilitySpec.SourceObject = this;

		DeathAbilitySpecHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
	else
	{
		UE_LOG(LogGAHA, Warning, TEXT("UHealthComponent::ApplyHealthData: DeathEventAbilityClass is not set in HealthData(%s). If you want to implement a character death event, you need to set."), *GetNameSafe(HealthData));
	}

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

			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, HealthData, this);

			CheckDefaultInitialization();
		}
	}
}


void UHealthComponent::AddDamageCauser(AActor* Causer, float Damage)
{
	auto& TotalDamage{ DamageCauserHistory.FindOrAdd(Causer) };
	TotalDamage += Damage;
}

void UHealthComponent::ClearDamageCauserHistory()
{
	DamageCauserHistory.Empty();
}

AActor* UHealthComponent::GetTopAssistCauser(AActor* FinalCauser) const
{
	AActor* Result{ nullptr };
	auto TopDamage{ 0.0f };

	for (const auto& KVP : DamageCauserHistory)
	{
		auto* Causer{ KVP.Key.Get() };
		const auto& Damage{ KVP.Value };

		if (Causer && (TopDamage < Damage) && (FinalCauser != Causer))
		{
			TopDamage = Damage;
			Result = Causer;
		}
	}

	return Result;
}


void UHealthComponent::OnRep_DeathState(EDeathState OldDeathState)
{
	const auto NewDeathState{ DeathState };

	// Revert the death state for now since we rely on StartDeath and FinishDeath to change it.

	DeathState = OldDeathState;

	// The server is trying to set us back but we've already predicted past the server state.

	if (OldDeathState > NewDeathState)
	{
		UE_LOG(LogGAHA, Warning, TEXT("UHealthComponent: Predicted past server death state [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		return;
	}

	if (OldDeathState == EDeathState::NotDead)
	{
		if (NewDeathState == EDeathState::DeathStarted)
		{
			HandleStartDeath();
		}
		else if (NewDeathState == EDeathState::DeathFinished)
		{
			HandleStartDeath();
			HandleFinishDeath();
		}
		else
		{
			UE_LOG(LogGAHA, Error, TEXT("UHealthComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}
	else if (OldDeathState == EDeathState::DeathStarted)
	{
		if (NewDeathState == EDeathState::DeathFinished)
		{
			HandleFinishDeath();
		}
		else
		{
			UE_LOG(LogGAHA, Error, TEXT("UHealthComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}

	if (DeathState != NewDeathState)
	{
		UE_LOG(LogGAHA, Error, TEXT("UHealthComponent: Death transition failed [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
	}
}

void UHealthComponent::HandleStartDeath()
{
	if (DeathState != EDeathState::NotDead)
	{
		return;
	}

	DeathState = EDeathState::DeathStarted;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Status_Death_Dying, 1);
	}

	auto* Owner{ GetOwner() };
	check(Owner);

	OnDeathStarted.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void UHealthComponent::HandleFinishDeath()
{
	if (DeathState != EDeathState::DeathStarted)
	{
		return;
	}

	DeathState = EDeathState::DeathFinished;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Status_Death_Dead, 1);
	}

	auto* Owner{ GetOwner() };
	check(Owner);

	OnDeathFinished.Broadcast(Owner);

	Owner->ForceNetUpdate();
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
	auto* Assister{ GetTopAssistCauser(DamageCauser) };

	// Sends a GameplayEvent to the AbilitySystemComponent of the Actor that owns this component.

	if (AbilitySystemComponent)
	{
		FGameplayEventData Payload;
		Payload.EventTag		= TAG_Event_OutOfHealth;
		Payload.Instigator		= DamageInstigator;
		Payload.Target			= AbilitySystemComponent->GetAvatarActor();
		Payload.OptionalObject	= Assister;
		Payload.ContextHandle	= DamageEffectSpec.GetEffectContext();
		Payload.InstigatorTags	= *DamageEffectSpec.CapturedSourceTags.GetAggregatedTags();
		Payload.TargetTags		= *DamageEffectSpec.CapturedTargetTags.GetAggregatedTags();
		Payload.EventMagnitude	= DamageMagnitude;

		auto NewScopedWindow{ FScopedPredictionWindow(AbilitySystemComponent, true) };
		AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
	}

	// Send messages to other systems through GameplayMessageSubsystem

	{
		FOutOfHealthMessage Message;
		Message.Instigator	= DamageInstigator;
		Message.Causer		= DamageCauser;
		Message.Assister	= Assister;
		Message.SourceTags	= *DamageEffectSpec.CapturedSourceTags.GetAggregatedTags();
		Message.TargetTags	= *DamageEffectSpec.CapturedTargetTags.GetAggregatedTags();
		Message.Damage		= DamageMagnitude;

		auto& MessageSystem{ UGameplayMessageSubsystem::Get(GetWorld()) };
		MessageSystem.BroadcastMessage(TAG_Message_OutOfHealth, Message);
	}

}

void UHealthComponent::HandleOnDamaged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{
	// Sends a GameplayEvent to the AbilitySystemComponent of the Actor that owns this component.

	if (AbilitySystemComponent)
	{
		FGameplayEventData Payload;
		Payload.EventTag		= TAG_Event_Damage;
		Payload.Instigator		= DamageInstigator;
		Payload.Target			= AbilitySystemComponent->GetAvatarActor();
		Payload.ContextHandle	= DamageEffectSpec.GetEffectContext();
		Payload.InstigatorTags	= *DamageEffectSpec.CapturedSourceTags.GetAggregatedTags();
		Payload.TargetTags		= *DamageEffectSpec.CapturedTargetTags.GetAggregatedTags();
		Payload.EventMagnitude	= DamageMagnitude;

		auto NewScopedWindow{ FScopedPredictionWindow(AbilitySystemComponent, true) };
		AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
	}

	// Send messages to other systems through GameplayMessageSubsystem

	{
		FHealthDamageMessage Message;
		Message.Instigator	= DamageInstigator;
		Message.Causer		= DamageCauser;
		Message.SourceTags	= *DamageEffectSpec.CapturedSourceTags.GetAggregatedTags();
		Message.TargetTags	= *DamageEffectSpec.CapturedTargetTags.GetAggregatedTags();
		Message.Damage		= DamageMagnitude;

		auto& MessageSystem{ UGameplayMessageSubsystem::Get(GetWorld()) };
		MessageSystem.BroadcastMessage(TAG_Message_Damage, Message);
	}

	AddDamageCauser(DamageCauser, DamageMagnitude);
}

void UHealthComponent::HandleOnHealed(AActor* HealInstigator, AActor* HealCauser, const FGameplayEffectSpec& HealEffectSpec, float HealMagnitude)
{
	// Sends a GameplayEvent to the AbilitySystemComponent of the Actor that owns this component.

	if (AbilitySystemComponent)
	{
		FGameplayEventData Payload;
		Payload.EventTag		= TAG_Event_Heal;
		Payload.Instigator		= HealInstigator;
		Payload.Target			= AbilitySystemComponent->GetAvatarActor();
		Payload.ContextHandle	= HealEffectSpec.GetEffectContext();
		Payload.InstigatorTags	= *HealEffectSpec.CapturedSourceTags.GetAggregatedTags();
		Payload.TargetTags		= *HealEffectSpec.CapturedTargetTags.GetAggregatedTags();
		Payload.EventMagnitude	= HealMagnitude;

		auto NewScopedWindow{ FScopedPredictionWindow(AbilitySystemComponent, true) };
		AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
	}

	// Send messages to other systems through GameplayMessageSubsystem

	{
		FHealthHealMessage Message;
		Message.Instigator	= HealInstigator;
		Message.Causer		= HealCauser;
		Message.SourceTags	= *HealEffectSpec.CapturedSourceTags.GetAggregatedTags();
		Message.TargetTags	= *HealEffectSpec.CapturedTargetTags.GetAggregatedTags();
		Message.Heal		= HealMagnitude;

		auto& MessageSystem{ UGameplayMessageSubsystem::Get(GetWorld()) };
		MessageSystem.BroadcastMessage(TAG_Message_Heal, Message);
	}

	ClearDamageCauserHistory();
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
