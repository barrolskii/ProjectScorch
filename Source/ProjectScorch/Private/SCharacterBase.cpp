// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacterBase.h"
#include "SWeaponBase.h"
#include "Classes/Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Classes/Engine/World.h"
#include "Components/SkeletalMeshComponent.h"


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

	AimDownSightsFOV = 65.0f;
	IsAimingDownSights = false;
	ZoomInterpSpeed = 20.0f;

	WeaponAttachSocketName = "WeaponSocket";
}

// Called when the game starts or when spawned
void ASCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = Camera->FieldOfView;

	// Spawn a default weapon
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentWeapon = GetWorld()->SpawnActor<ASWeaponBase>(StarterWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
	}
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

void ASCharacterBase::Fire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Fire();
	}
}

// Called every frame
void ASCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = IsAimingDownSights ? AimDownSightsFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(Camera->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

	Camera->SetFieldOfView(NewFOV);
}

FVector ASCharacterBase::GetPawnViewLocation() const
{
	if (Camera)
	{
		return Camera->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
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

	PlayerInputComponent->BindAction("AimDownSights", IE_Pressed, this, &ASCharacterBase::BeginAimDownSights);
	PlayerInputComponent->BindAction("AimDownSights", IE_Released, this, &ASCharacterBase::EndAimDownSights);
	PlayerInputComponent->BindAction("FireWeapon", IE_Pressed, this, &ASCharacterBase::Fire);
}

