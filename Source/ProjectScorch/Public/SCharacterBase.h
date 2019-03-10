// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacterBase.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class PROJECTSCORCH_API ASCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float value);
	void MoveRight(float value);

	void BeginCrouch();
	void EndCrouch();

	FORCEINLINE void BeginAimDownSights() { IsAimingDownSights = true; }
	FORCEINLINE void EndAimDownSights() { IsAimingDownSights = false;  }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UCameraComponent *Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	USpringArmComponent *SpringArm;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float AimDownSightsFOV;
	
	float DefaultFOV;
	
	bool IsAimingDownSights;

	UPROPERTY(EditDefaultsOnly, Category = "Player", Meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual FVector GetPawnViewLocation() const override;
};
