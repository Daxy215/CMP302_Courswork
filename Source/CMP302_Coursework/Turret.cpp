// Fill out your copyright notice in the Description page of Project Settings.


#include "Turret.h"

#include "CMP302_CourseworkCharacter.h"
#include "CMP302_CourseworkProjectile.h"
#include "Kismet/GameplayStatics.h"

#include "Engine/StaticMesh.h"

// Sets default values
ATurret::ATurret()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ATurret::Fire() {
	// Try and fire a projectile
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			APlayerController* PlayerController = Cast<APlayerController>(characterMovement->GetController());
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
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, characterMovement->GetActorLocation());
	}
}

// Called when the game starts or when spawned
void ATurret::BeginPlay()
{
	Super::BeginPlay();
	
	characterMovement = Cast<ACharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), ACMP302_CourseworkCharacter::StaticClass()));
}

// Called every frame
void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector TargetLocation = characterMovement->GetTransform().GetLocation();
	
	float Distance = FVector::Distance(TargetLocation, GetTransform().GetLocation());
	
	if(Distance > LookAtDistance)
		return;
	
	// Calculate the direction to the target location
	FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal2D(); // GetSafeNormal2D for 2D rotation
	Direction.Z = 0; // Optionally, ignore Z-axis if you want the actor to look horizontally
	
	// Calculate the rotation needed to face the target
	FRotator TargetRotation = Direction.Rotation();

	// Interpolate rotation gradually each frame
	FRotator CurrentRotation = GetActorRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);

	// Set the actor's rotation to the new interpolated rotation
	SetActorRotation(NewRotation);

	if(Distance > ShootDistance)
		return;
	
	Fire();
}

