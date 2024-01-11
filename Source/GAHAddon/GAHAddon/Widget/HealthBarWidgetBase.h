// Copyright (C) 2024 owoDra

#pragma once

#include "Blueprint/UserWidget.h"

#include "HealthBarWidgetBase.generated.h"

class UHealthComponent;
class APawn;
class AActor;


/**
 * Base widget class that automatically observes HealthComponent changes and provides basic functionality for health bar implementation
 */
UCLASS(Abstract, Blueprintable)
class UHealthBarWidgetBase : public UUserWidget
{
	GENERATED_BODY()
public:
	UHealthBarWidgetBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	void ListenPawnChange();
	void UnlistenPawnChange();

private:
	UFUNCTION()
	void HandlePawnChange(APawn* OldPawn, APawn* NewPawn);


protected:
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Components")
	TWeakObjectPtr<UHealthComponent> HealthComponent;

protected:
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void RefreshHealthComponent(APawn* InPawn);

	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void RefreshHealthValues();

	void ListenHealthEvents();
	void UnlistenHealthEvents();


private:
	UFUNCTION()
	void HandleHealthChanged(UHealthComponent* InHealthComponent, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION()
	void HandleMaxHealthChanged(UHealthComponent* InHealthComponent, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION()
	void HandleMinHealthChanged(UHealthComponent* InHealthComponent, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION()
	void HandleExtraHealthChanged(UHealthComponent* InHealthComponent, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION()
	void HandleShieldChanged(UHealthComponent* InHealthComponent, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION()
	void HandleMaxShieldChanged(UHealthComponent* InHealthComponent, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION()
	void HandleDeath(AActor* OwningActor);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnHealthChanged(float NewValue, float OldValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnMaxHealthChanged(float NewValue, float OldValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnMinHealthChanged(float NewValue, float OldValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnExtraHealthChanged(float NewValue, float OldValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnShieldChanged(float NewValue, float OldValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnMaxShieldChanged(float NewValue, float OldValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnDeath();

	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnRevive();

};
