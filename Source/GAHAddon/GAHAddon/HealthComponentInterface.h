// Copyright (C) 2023 owoDra

#pragma once

#include "UObject/Interface.h"

#include "HealthComponentInterface.generated.h"

class UHealthComponent;


/** 
 * Interface for actors that expose access to an health component 
 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UHealthComponentInterface : public UInterface
{
	GENERATED_BODY()
public:
	UHealthComponentInterface(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

};

class GAHADDON_API IHealthComponentInterface
{
	GENERATED_BODY()
public:
	IHealthComponentInterface() {}

public:
	/** 
	 * Returns the health component to use for this actor. It may live on another actor, such as a Pawn using the PlayerState's component 
	 */
	virtual UHealthComponent* GetHealthComponent() const = 0;

};
