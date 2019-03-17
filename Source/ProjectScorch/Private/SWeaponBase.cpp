// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Classes/PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "ProjectScorch.h"
#include "Net/UnrealNetwork.h"

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

	BaseDamage = 20.0f;
	FireRate = 600;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void ASWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / FireRate;
}

void ASWeaponBase::BeginFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(ShotTimer, this, &ASWeaponBase::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeaponBase::EndFire()
{
	GetWorldTimerManager().ClearTimer(ShotTimer);
}

void ASWeaponBase::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

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
		QueryParams.bReturnPhysicalMaterial = true;

		// Particle "Target" parameter
		FVector TracerEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		FHitResult HitData;
		if (GetWorld()->LineTraceSingleByChannel(HitData, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			// Process what the line hit
			AActor *HitActor = HitData.GetActor();

			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitData.PhysMaterial.Get());
			
			float AppliedDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESH_VULNERABLE)
			{
				AppliedDamage *= 2.0f;
			}

			
			UGameplayStatics::ApplyPointDamage(HitActor, AppliedDamage, ShotDirection, HitData, Owner->GetInstigatorController(), this, DamageType);

			PlayImpactEffects(SurfaceType, HitData.ImpactPoint);

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

		if (Role == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

void ASWeaponBase::OnRep_HitScanTrace()
{
	// Play cosmetic effects
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
	PlayMuzzleEffect();
	PlayTracerEffect(&HitScanTrace.TraceTo);
}

void ASWeaponBase::ServerFire_Implementation()
{
	Fire();
}

bool ASWeaponBase::ServerFire_Validate()
{
	return true;
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

void ASWeaponBase::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem *SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESH_DEFAULT:
	case SURFACE_FLESH_VULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void ASWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeaponBase, HitScanTrace, COND_SkipOwner);
}