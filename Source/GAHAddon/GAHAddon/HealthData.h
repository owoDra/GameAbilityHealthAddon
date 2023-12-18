// Copyright (C) 2023 owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "HealthData.generated.h"


/**
 * The data of the maximum default health and the amount of shield.
 */
UCLASS(BlueprintType, Const)
class GAHADDON_API UHealthData : public UDataAsset
{
	GENERATED_BODY()
public:
	UHealthData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxHealth{ 100.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MinHealth{ 0.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Health{ 100.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ExtraHealth{ 0.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxShield{ 50.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Shield{ 50.0f };

};
