// Copyright (C) 2024 owoDra

#include "HealthBarWidgetBase.h"

#include "HealthFunctionLibrary.h"
#include "HealthComponent.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HealthBarWidgetBase)


UHealthBarWidgetBase::UHealthBarWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UHealthBarWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ListenPawnChange();

	RefreshHealthComponent(GetOwningPlayerPawn());
}

void UHealthBarWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();

	UnlistenHealthEvents();
	UnlistenPawnChange();
}


void UHealthBarWidgetBase::ListenPawnChange()
{
	auto* PlayerController{ GetOwningPlayer() };

	if (ensure(PlayerController))
	{
		FScriptDelegate NewDelegate;
		NewDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UHealthBarWidgetBase, HandlePawnChange));
		PlayerController->OnPossessedPawnChanged.Add(NewDelegate);
	}
}

void UHealthBarWidgetBase::UnlistenPawnChange()
{
	auto* PlayerController{ GetOwningPlayer() };

	if (ensure(PlayerController))
	{
		PlayerController->OnPossessedPawnChanged.RemoveAll(this);
	}
}

void UHealthBarWidgetBase::HandlePawnChange(APawn* OldPawn, APawn* NewPawn)
{
	RefreshHealthComponent(NewPawn);
}


void UHealthBarWidgetBase::RefreshHealthComponent(APawn* InPawn)
{
	if (auto* NewHealthComponent{ UHealthFunctionLibrary::GetHealthComponentFromActor(InPawn) })
	{
		UnlistenHealthEvents();

		HealthComponent = NewHealthComponent;

		ListenHealthEvents();

		RefreshHealthValues();
	}
}

void UHealthBarWidgetBase::RefreshHealthValues()
{
	if (HealthComponent.IsValid())
	{
		const auto Health{ HealthComponent->GetHealth() };
		OnHealthChanged(Health, Health);

		const auto MaxHealth{ HealthComponent->GetMaxHealth() };
		OnMaxHealthChanged(MaxHealth, MaxHealth);

		const auto MinHealth{ HealthComponent->GetMinHealth() };
		OnMinHealthChanged(MinHealth, MinHealth);

		const auto ExtraHealth{ HealthComponent->GetExtraHealth() };
		OnExtraHealthChanged(ExtraHealth, ExtraHealth);

		const auto Shield{ HealthComponent->GetShield() };
		OnShieldChanged(Shield, Shield);

		const auto MaxShield{ HealthComponent->GetMaxShield() };
		OnMaxShieldChanged(MaxShield, MaxShield);

		if (Health > 0.0f)
		{
			OnRevive();
		}
		else
		{
			OnDeath();
		}
	}
}

void UHealthBarWidgetBase::ListenHealthEvents()
{
	if (HealthComponent.IsValid())
	{
		{
			FScriptDelegate NewDelegate;
			NewDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UHealthBarWidgetBase, HandleHealthChanged));
			HealthComponent->OnHealthChanged.Add(NewDelegate);
		}

		{
			FScriptDelegate NewDelegate;
			NewDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UHealthBarWidgetBase, HandleMaxHealthChanged));
			HealthComponent->OnMaxHealthChanged.Add(NewDelegate);
		}

		{
			FScriptDelegate NewDelegate;
			NewDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UHealthBarWidgetBase, HandleMinHealthChanged));
			HealthComponent->OnMinHealthChanged.Add(NewDelegate);
		}

		{
			FScriptDelegate NewDelegate;
			NewDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UHealthBarWidgetBase, HandleExtraHealthChanged));
			HealthComponent->OnExtraHealthChanged.Add(NewDelegate);
		}

		{
			FScriptDelegate NewDelegate;
			NewDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UHealthBarWidgetBase, HandleShieldChanged));
			HealthComponent->OnShieldChanged.Add(NewDelegate);
		}

		{
			FScriptDelegate NewDelegate;
			NewDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UHealthBarWidgetBase, HandleMaxShieldChanged));
			HealthComponent->OnMaxShieldChanged.Add(NewDelegate);
		}

		{
			FScriptDelegate NewDelegate;
			NewDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UHealthBarWidgetBase, HandleDeath));
			HealthComponent->OnDeathStarted.Add(NewDelegate);
		}
	}
}

void UHealthBarWidgetBase::UnlistenHealthEvents()
{
	if (HealthComponent.IsValid())
	{
		HealthComponent->OnHealthChanged.RemoveAll(this);
		HealthComponent->OnMaxHealthChanged.RemoveAll(this);
		HealthComponent->OnMinHealthChanged.RemoveAll(this);
		HealthComponent->OnExtraHealthChanged.RemoveAll(this);
		HealthComponent->OnShieldChanged.RemoveAll(this);
		HealthComponent->OnMaxShieldChanged.RemoveAll(this);
		HealthComponent->OnDeathStarted.RemoveAll(this);
	}
}


void UHealthBarWidgetBase::HandleHealthChanged(UHealthComponent* InHealthComponent, float OldValue, float NewValue, AActor* Instigator)
{
	OnHealthChanged(NewValue, OldValue);
}

void UHealthBarWidgetBase::HandleMaxHealthChanged(UHealthComponent* InHealthComponent, float OldValue, float NewValue, AActor* Instigator)
{
	OnMaxHealthChanged(NewValue, OldValue);
}

void UHealthBarWidgetBase::HandleMinHealthChanged(UHealthComponent* InHealthComponent, float OldValue, float NewValue, AActor* Instigator)
{
	OnMinHealthChanged(NewValue, OldValue);
}

void UHealthBarWidgetBase::HandleExtraHealthChanged(UHealthComponent* InHealthComponent, float OldValue, float NewValue, AActor* Instigator)
{
	OnExtraHealthChanged(NewValue, OldValue);
}

void UHealthBarWidgetBase::HandleShieldChanged(UHealthComponent* InHealthComponent, float OldValue, float NewValue, AActor* Instigator)
{
	OnShieldChanged(NewValue, OldValue);
}

void UHealthBarWidgetBase::HandleMaxShieldChanged(UHealthComponent* InHealthComponent, float OldValue, float NewValue, AActor* Instigator)
{
	OnMaxShieldChanged(NewValue, OldValue);
}

void UHealthBarWidgetBase::HandleDeath(AActor* OwningActor)
{
	OnDeath();
}
