// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ASWeaponBase::ASWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";
}

// Called when the game starts or when spawned
void ASWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASWeaponBase::Fire()
{
	// Trace a line from pawn eyes to the crosshair
	UE_LOG(LogTemp, Log, TEXT("Fire function called"));


	AActor *Owner = GetOwner();
	if (Owner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Owner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;

		// Particle "Target" parameter
		FVector TracerEndPoint = TraceEnd;

		FHitResult HitData;
		if (GetWorld()->LineTraceSingleByChannel(HitData, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
		{
			// Process what the line hit
			AActor *HitActor = HitData.GetActor();

			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, HitData, Owner->GetInstigatorController(), this, DamageType);

			if (ImpactEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, HitData.ImpactPoint, HitData.ImpactNormal.Rotation());
			}
			
			TracerEndPoint = HitData.ImpactPoint;
		}


		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);

		if (MuzzleEffect)
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
		}


		if (TracerEffect)
		{
			FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
			UParticleSystemComponent *TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
			
			if (TracerComp)
			{
				TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
			}
		}
	}
}