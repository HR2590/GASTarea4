// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilityBase_Busy.h"
#include "AbilityDodge.generated.h"

/**
 * 
 */
UCLASS()
class TAREA3GAS_API UAbilityDodge : public UAbilityBase_Busy
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Abilities")
	float DodgeStrength = 3000.f;
	
	UFUNCTION(Blueprintable,BlueprintCallable,Category="Abilities|OnAnimationFinished")
	void OnAnimationFinished();
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	
};
