// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tarea3GASPlayerController.h"

#include "Abilities//AbilityTargetEnemy.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Tarea3GASCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UTHUB_ASC.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ATarea3GASPlayerController::ATarea3GASPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
}

void ATarea3GASPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

void ATarea3GASPlayerController::StartedAbility(const FInputActionInstance& InputActionInstance)
{
	if(!IsValid(GetPawn())||GetPawn()->IsPendingKillPending()) return;
	UGASDataComponent* DataComponent=GetPawn()->FindComponentByClass<UGASDataComponent>();
	UUTHUB_ASC* ASC=GetPawn()->FindComponentByClass<UUTHUB_ASC>();
	
	if(DataComponent && ASC&&DataComponent->InputAbilityMapping )
	{
		const UInputAction* Action = InputActionInstance.GetSourceAction();
		for (auto [InputID,InputMap]:DataComponent->InputAbilityMapping->InputMap)
		{
			const UInputAction* InAction=InputActionInstance.GetSourceAction();
			if(const auto ActionClass= InputMap.AbilityMap.Find(InAction))
			{
				const auto Spec=ASC->FindAbilitySpecFromClass(*ActionClass);
				if(ASC->TryActivateAbilityByClass(*ActionClass))
					ASC->AbilityLocalInputPressed(static_cast<uint8>(InputID));
				
					
			}
		}
	}
}

void ATarea3GASPlayerController::CompletedAbility(const FInputActionInstance& InputActionInstance)
{
	if(!IsValid(GetPawn())||GetPawn()->IsPendingKillPending()) return;
	UGASDataComponent* DataComponent=GetPawn()->FindComponentByClass<UGASDataComponent>();
	UUTHUB_ASC* ASC=GetPawn()->FindComponentByClass<UUTHUB_ASC>();
	
	if(DataComponent && ASC&&DataComponent->InputAbilityMapping )
	{
		const UInputAction* Action = InputActionInstance.GetSourceAction();
		for (auto [InputID,InputMap]:DataComponent->InputAbilityMapping->InputMap)
		{
			const UInputAction* InAction=InputActionInstance.GetSourceAction();
			if(const auto ActionClass= InputMap.AbilityMap.Find(InAction))
			{
				const auto Spec=ASC->FindAbilitySpecFromClass(*ActionClass);
				
				if(ASC->TryActivateAbilityByClass(*ActionClass))
					ASC->AbilityLocalInputReleased(static_cast<uint8>(InputID));
			}
		}
	}
}

void ATarea3GASPlayerController::OnCancelLockOn()
{
	UAbilitySystemComponent* ASC = GetPawn()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC) return;
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetClass()->IsChildOf(UAbilityTargetEnemy::StaticClass()))
		{
			ASC->CancelAbility(Spec.Ability);
		}
	}
}

void ATarea3GASPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Setup mouse input events
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &ATarea3GASPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &ATarea3GASPlayerController::OnSetDestinationTriggered);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &ATarea3GASPlayerController::OnSetDestinationReleased);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &ATarea3GASPlayerController::OnSetDestinationReleased);

		// Setup touch input events
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &ATarea3GASPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &ATarea3GASPlayerController::OnTouchTriggered);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &ATarea3GASPlayerController::OnTouchReleased);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &ATarea3GASPlayerController::OnTouchReleased);
		
		
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATarea3GASPlayerController::OnInputStarted()
{
	StopMovement();
}

// Triggered every frame when the input is held down
void ATarea3GASPlayerController::OnSetDestinationTriggered()
{
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	
	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = false;
	if (bIsTouch)
	{
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, ECollisionChannel::ECC_Visibility, true, Hit);
	}
	else
	{
		bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}
	
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void ATarea3GASPlayerController::OnSetDestinationReleased()
{
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

// Triggered every frame when the input is held down
void ATarea3GASPlayerController::OnTouchTriggered()
{
	bIsTouch = true;
	OnSetDestinationTriggered();
}

void ATarea3GASPlayerController::OnTouchReleased()
{
	bIsTouch = false;
	OnSetDestinationReleased();
}



void ATarea3GASPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UEnhancedInputComponent* EnhancedInputComponent=Cast<UEnhancedInputComponent>(InputComponent);
	if(!IsValid(GetPawn())||GetPawn()->IsPendingKillPending()) return;
	
		UUTHUB_ASC* ASC=GetPawn()->FindComponentByClass<UUTHUB_ASC>();
		if(EnhancedInputComponent&&ASC)
		{
			if(const UGASDataComponent* DataComponent=InPawn->FindComponentByClass<UGASDataComponent>())
			{
				if(DataComponent->InputAbilityMapping)
				{
					for (auto [InputID,InputMap]: DataComponent->InputAbilityMapping->InputMap)
					{
						for (auto [InputAction,AbilityClass] : InputMap.AbilityMap)
						{
							if (InputAction&&AbilityClass)
							{
								ASC->AddAbilityFromClass(AbilityClass,static_cast<uint8>(InputID));
							
								if(InputID==EAbilityInputID::Defend)
								{
									EnhancedInputComponent->BindAction(InputAction,ETriggerEvent::Triggered,this,&ThisClass::StartedAbility);
									EnhancedInputComponent->BindAction(InputAction,ETriggerEvent::Completed,this,&ThisClass::CompletedAbility);
								}
								if(InputID==EAbilityInputID::Jump)
								{
									EnhancedInputComponent->BindAction(InputAction,ETriggerEvent::Started,this,&ThisClass::StartedAbility);
								}
								else
								{
									EnhancedInputComponent->BindAction(InputAction,ETriggerEvent::Triggered,this,&ThisClass::StartedAbility);
								}
						
								
							}
							if (InputID==EAbilityInputID::CancelTarget)
							{
								EnhancedInputComponent->BindAction(InputAction,ETriggerEvent::Started,this,&ThisClass::OnCancelLockOn);
							}
						}
					
						
					}
				}
				
			}
					
				
		}
	
	
		
}



	

