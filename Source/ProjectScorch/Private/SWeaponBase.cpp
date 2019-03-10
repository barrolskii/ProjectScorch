// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("Scorch.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draws debug lines for weapons"),
	ECVF_Cheat);

// Sets default values
ASWeaponBase::ASWeaponBase()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";
}

void ASWeaponBase::Fire()
{
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

		if (DebugWeaponDrawing > 0)
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);

		
		PlayMuzzleEffect();
		PlayTracerEffect(&TracerEndPoint);

		APawn *POwner = Cast<APawn>(GetOwner());
		if (POwner)
		{
			APlayerController *PC = Cast<APlayerController>(POwner->GetController());
			if (PC)
			{
				PC->ClientPlayCameraShake(FireCameraShake);
			}
		}
	}
}

void ASWeaponBase::PlayMuzzleEffect()
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}
}

void ASWeaponBase::PlayTracerEffect(FVector *TracerEndPoint)
{
	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent *TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);

		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, *TracerEndPoint);
		}
	}
}