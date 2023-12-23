// Copyright (C) 2023 owoDra

#pragma once

#include "GameplayTagContainer.h"

#include "HealthMessageTypes.generated.h"


/**
 * Data for message notifying that HP is no longer available
 */
USTRUCT(BlueprintType)
struct FOutOfHealthMessage
{
	GENERATED_BODY()
public:
	FOutOfHealthMessage() {}

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<AActor> Instigator{ nullptr };

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<AActor> Causer{ nullptr };

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<AActor> Assister{ nullptr };

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer SourceTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer TargetTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Damage{ 0.0f };

};


/**
 * Data for messages notifying the user that damage has been inflicted.
 */
USTRUCT(BlueprintType)
struct FHealthDamageMessage
{
	GENERATED_BODY()
public:
	FHealthDamageMessage() {}

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<AActor> Instigator{ nullptr };

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<AActor> Causer{ nullptr };

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer SourceTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer TargetTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Damage{ 0.0f };

};


/**
 * Data for messages notifying the user that heal has been inflicted.
 */
USTRUCT(BlueprintType)
struct FHealthHealMessage
{
	GENERATED_BODY()
public:
	FHealthHealMessage() {}

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<AActor> Instigator{ nullptr };

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<AActor> Causer{ nullptr };

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer SourceTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer TargetTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Heal{ 0.0f };

};
