// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "CMP302_CourseworkCharacter.h"
#include "CMP302_CourseworkProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
}


void UTP_WeaponComponent::Fire()
{
	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}

	// Try and fire a projectile
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);
	
			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	
			// Spawn the projectile at the muzzle
			World->SpawnActor<ACMP302_CourseworkProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}
	
	// Try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}
	
	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void UTP_WeaponComponent::AttachWeapon(ACMP302_CourseworkCharacter* TargetCharacter)
{
	Character = TargetCharacter;
	if (Character == nullptr)
	{
		return;
	}

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));
	
	// switch bHasRifle so the animation blueprint can switch to another animation set
	Character->SetHasRifle(true);
	Character->weapon = this;

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			// Fire
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::Fire);
			
			EnhancedInputComponent->BindAction(GrapplingHookFireAction, ETriggerEvent::Started, this, &UTP_WeaponComponent::FireGrapplingHook);
			EnhancedInputComponent->BindAction(GrapplingHookFireAction, ETriggerEvent::Completed, this, &UTP_WeaponComponent::StopGrapplingHook);
		}
	}
}

void UTP_WeaponComponent::FireGrapplingHook(const FInputActionValue& Value)
{
	if(Timer < GrapplingHookCooldown)
		return;
	
	Timer = 0;
	
	// Get the player controller
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	
	if (PlayerController)
	{
		// Get the camera location and forward vector to start the trace
		FVector CameraLocation;
		FRotator CameraRotation;
		PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
		
		// Calculate the end of the trace by extending the forward vector
		FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * 3000); // Set MaxTraceDistance as desired
		
		FHitResult HitResult;
		
		// Set collision parameters
		FCollisionQueryParams TraceParams(FName(TEXT("Trace")), true, GetOwner());
		
		// Perform the line trace
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_Visibility, TraceParams);
		
		// Check if the trace hit something
		if (bHit)
		{
			GrapplingEndPosition = HitResult.ImpactPoint;
			IsGrappling = true;
			
			// Set the movement mode to flying
			Character->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		}
	}
}

void UTP_WeaponComponent::StopGrapplingHook(const FInputActionValue& Value)
{
	if(!IsGrappling)
		return;
	
	IsGrappling = false;
	
	// Set the movement mode to falling
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
}

void UTP_WeaponComponent::Update(float DeltaSeconds) {
	Timer += DeltaSeconds;	
	
	if(!IsGrappling)
		return;
	
	UCharacterMovementComponent* MyCharacterMovement = Character->GetCharacterMovement();
	FVector ActorLocation = MyCharacterMovement->GetActorLocation();
	FVector DirectionVector = (GrapplingEndPosition - ActorLocation).GetSafeNormal();
	FVector ForwardVector = MyCharacterMovement->GetActorLocation().ForwardVector;
	FVector UpVector = MyCharacterMovement->GetActorLocation().UpVector;
	FVector MoveRightVector = FVector::CrossProduct(ForwardVector, UpVector);
	FVector RightVector = MyCharacterMovement->GetActorLocation().RightVector;
	FVector OverallVector = (RightVector * MoveRightVector) * 0.4f;
	
	float Distance = FVector::Distance(ActorLocation, Character->GetTransform().GetLocation());
	
	DirectionVector += OverallVector;
	DirectionVector.Normalize();
	DirectionVector *= GrapplingHookSpeed + (Distance * 100); //Speed
	
	MyCharacterMovement->AddForce(DirectionVector);
}

void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Character == nullptr)
	{
		return;
	}
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FireMappingContext);
		}
	}
}
