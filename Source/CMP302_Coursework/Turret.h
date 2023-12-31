// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Turret.generated.h"

class UStaticMesh;

UCLASS()
class CMP302_COURSEWORK_API ATurret : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATurret();

	void Fire(FRotator TargetDirection);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class ACMP302_CourseworkProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	ACharacter* CharacterMovement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ShootPosition;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret Properties",  meta = (AllowPrivateAccess = "true"))
    float FireRate = 2;
	float Timer;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret Properties",  meta = (AllowPrivateAccess = "true"))
	float RotationSpeed = 2;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret Properties",  meta = (AllowPrivateAccess = "true"))
	float LookAtDistance = 1500;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret Properties",  meta = (AllowPrivateAccess = "true"))
	float ShootDistance = 1000;
};
