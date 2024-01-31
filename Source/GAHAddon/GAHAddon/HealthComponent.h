// Copyright (C) 2024 owoDra

#pragma once

#include "Component/GFCActorComponent.h"

#include "GameplayAbilitySpec.h"

#include "HealthComponent.generated.h"

class UAbilitySystemComponent;
class UHealthAttributeSet;
class UCombatAttributeSet;
class UHealthData;
struct FGameplayEffectSpec;
struct FOnAttributeChangeData;


/**
 * Delegate for actor's death-related events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathDelegate, AActor*, OwningActor);

/**
 * Delegate used to notify that the value of an attribute, such as actor health, has changed
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthAttributeChangedDelegate, UHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);


/**
 * Indicates that the actor is dead or in a death state
 */
UENUM(BlueprintType)
enum class EDeathState : uint8
{
	NotDead = 0,
	DeathStarted,
	DeathFinished,
};


/**
 * Components that manage actor health, shield, death state and more.
 */
UCLASS(Meta=(BlueprintSpawnableComponent))
class GAHADDON_API UHealthComponent : public UGFCActorComponent
{
	GENERATED_BODY()
public:
	UHealthComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//
	// Function name used to add this component
	//
	static const FName NAME_ActorFeatureName;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent{ nullptr };

	UPROPERTY(Transient)
	TObjectPtr<const UHealthAttributeSet> HealthSet{ nullptr };

	UPROPERTY(Transient)
	TObjectPtr<const UCombatAttributeSet> CombatSet{ nullptr };

	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle DeathAbilitySpecHandle;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeWithAbilitySystem();
	void UninitializeFromAbilitySystem();
	void ClearGameplayTags();

public:
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;

protected:
	virtual bool CanChangeInitStateToDataInitialized(UGameFrameworkComponentManager* Manager) const override;
	virtual void HandleChangeInitStateToDataInitialized(UGameFrameworkComponentManager* Manager) override;

	
protected:
	//
	// Current health data
	//
	UPROPERTY(EditAnywhere, ReplicatedUsing = "OnRep_HealthData")
	TObjectPtr<const UHealthData> HealthData{ nullptr };

protected:
	UFUNCTION()
	virtual void OnRep_HealthData();

	/**
	 * Apply the current health data
	 */
	virtual void ApplyHealthData();

	/**
	 * Calls when health data set or changed
	 */
	virtual void HandleHealthDataUpdated();

	/**
	 * Remove registered death abilities from the abilities system
	 */
	virtual void RemoveDeathAbilityFromSystem();

public:
	/**
	 * Set the current health data
	 */
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void SetHealthData(const UHealthData* NewHealthData);


protected:
	//
	// List of damaged opponents and amount of damage
	//
	TMap<TSoftObjectPtr<AActor>, float> DamageCauserHistory;

protected:
	/**
	 * Add the Actor who has done the damage.
	 */
	void AddDamageCauser(AActor* Causer, float Damage);

	/**
	 * Clear damage causer history
	 */
	void ClearDamageCauserHistory();

private:
	/**
	 * Returns the actor who contributed the most, excluding the actor who did the last damage
	 */
	AActor* GetTopAssistCauser(AActor* FinalCauser) const;


protected:
	//
	// Current death state
	//
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	EDeathState DeathState{ EDeathState::NotDead };

protected:
	UFUNCTION()
	virtual void OnRep_DeathState(EDeathState OldDeathState);

public:
	/**
	 * Executed when the death process is started
	 */
	virtual void HandleStartDeath();

	/**
	 * Executed when the death process is finished
	 */
	virtual void HandleFinishDeath();

public:
	/**
	 * Returns current death state
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	EDeathState GetDeathState() const { return DeathState; }

	/**
	 * Returns is dead or dying
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Health", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsDeadOrDying() const { return (DeathState > EDeathState::NotDead); }

	
public:
	UPROPERTY(BlueprintAssignable)
	FOnHealthAttributeChangedDelegate OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnHealthAttributeChangedDelegate OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnHealthAttributeChangedDelegate OnMinHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnHealthAttributeChangedDelegate OnExtraHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnHealthAttributeChangedDelegate OnShieldChanged;

	UPROPERTY(BlueprintAssignable)
	FOnHealthAttributeChangedDelegate OnMaxShieldChanged;

	UPROPERTY(BlueprintAssignable)
	FOnDeathDelegate OnDeathStarted;

	UPROPERTY(BlueprintAssignable)
	FOnDeathDelegate OnDeathFinished;

protected:
	virtual void HandleHealthChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleMaxHealthChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleMinHealthChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleExtraHealthChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleShieldChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleMaxShieldChanged(const FOnAttributeChangeData& ChangeData);

	virtual void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude);
	virtual void HandleOnDamaged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude);
	virtual void HandleOnHealed(AActor* HealInstigator, AActor* HealCauser, const FGameplayEffectSpec& HealEffectSpec, float HealMagnitude);
	
public:
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMinHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetExtraHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetShield() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxShield() const;

	/**
	 * Returns the combined value of current health, extra health, and shield.
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetTotalHealth() const;

	/**
	 * Returns the combined value of current max health, extra health, and max shield.
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetTotalMaxHealth() const;


public:
	UFUNCTION(BlueprintPure, Category = "Component")
	static UHealthComponent* FindHealthComponent(const AActor* Actor);

};
