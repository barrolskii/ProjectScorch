// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacterBase.h"
#include "Classes/Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"

// Sets default values
ASCharacterBase::ASCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SetupAttachment(RootComponent);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

// Called when the game starts or when spawned
void ASCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASCharacterBase::MoveForward(float value)
{
	AddMovementInput(GetActorForwardVector() * value);
}

void ASCharacterBase::MoveRight(float value)
{
	AddMovementInput(GetActorRightVector() * value);
}

void ASCharacterBase::BeginCrouch()
{
	Crouch();
}

void ASCharacterBase::EndCrouch()
{
	UnCrouch();
}

// Called every frame
void ASCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacterBase::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacterBase::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacterBase::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacterBase::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacterBase::EndCrouch);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
}

