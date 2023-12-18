// Copyright (C) 2023 owoDra

#include "HealthFunctionLibrary.h"

#include "HealthComponent.h"
#include "HealthComponentInterface.h"

#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HealthFunctionLibrary)


UHealthComponent* UHealthFunctionLibrary::GetHealthComponentFromActor(const AActor* Actor, bool LookForComponent)
{
	if (!Actor)
	{
		return nullptr;
	}

	const auto* HCI{ Cast<IHealthComponentInterface>(Actor) };
	if (HCI)
	{
		return HCI->GetHealthComponent();
	}

	if (LookForComponent)
	{
		return UHealthComponent::FindHealthComponent(Actor);
	}

	return nullptr;
}
