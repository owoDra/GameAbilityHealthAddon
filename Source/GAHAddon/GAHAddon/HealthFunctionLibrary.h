// Copyright (C) 2024 owoDra

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "HealthFunctionLibrary.generated.h"

class UHealthComponent;
class AActor;


UCLASS(MinimalAPI)
class UHealthFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/**
	 * Get HealthComponent from actor
	 */
	UFUNCTION(BlueprintPure, Category = "Health", meta = (BlueprintInternalUseOnly = "false"))
	static GAHADDON_API UHealthComponent* GetHealthComponentFromActor(const AActor* Actor, bool LookForComponent = true);

};
