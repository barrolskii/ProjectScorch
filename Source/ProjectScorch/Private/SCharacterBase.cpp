// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacterBase.h"
#include "SWeaponBase.h"
#include "Classes/Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Classes/Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "ProjectScorch.h"
#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASCharacterBase::ASCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SetupAttachment(RootComponent);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));


	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

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
	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacterBase::OnHealthChanged);

	if (Role == ROLE_Authority)
	{
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

void ASCharacterBase::BeginFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->BeginFire();
	}
}

void ASCharacterBase::EndFire()
{
	if (CurrentWeapon)
		CurrentWeapon->EndFire();
}

void ASCharacterBase::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, 
									  float HealthDelta, const class UDamageType* DamageType, 
									  class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied)
	{
		// Kill the character
		bDied = true;

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();

		SetLifeSpan(10.0f);
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

	PlayerInputComponent->BindAction("FireWeapon", IE_Pressed, this, &ASCharacterBase::BeginFire);
	PlayerInputComponent->BindAction("FireWeapon", IE_Released, this, &ASCharacterBase::EndFire);
}

/* Specify what and how we want to replicate objects for multiplayer */
void ASCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacterBase, CurrentWeapon);
	DOREPLIFETIME(ASCharacterBase, bDied);
}