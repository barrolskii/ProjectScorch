// Fill out your copyright notice in the Description page of Project Settings.

#include "SExplosiveBarrel.h"
#include "Components/StaticMeshComponent.h"
#include "Classes/PhysicsEngine/RadialForceComponent.h"
#include "SHealthComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody); // Allows to be affected by other barrels exploding
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);
	
	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("ForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = 250;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false; // Stop the component from ticking and use FireImpulse() instead
	RadialForceComp->bIgnoreOwningActor = true; // Ignore self

	ExplosionImpulse = 400;

	IsDestroyed = false;

	SetReplicates(true);
	SetReplicateMovement(true);
}

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, 
										float HealthDelta, const class UDamageType* DamageType, 
										class AController* InstigatedBy, AActor* DamageCauser)
{
	if (IsDestroyed) return; // Actor has already been destroy so do nothing

	if (Health <= 0.0f)
	{
		IsDestroyed = true;

		/* Call client explode method */
		OnRep_PlayEffect();

		// Boost the barrel upwards
		FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
		MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

		// Play the effect and change the actors material to the damaged material
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
		MeshComp->SetMaterial(0, DestroyedMaterial);

		RadialForceComp->FireImpulse();
	}
}

void ASExplosiveBarrel::OnRep_PlayEffect()
{
	// Play the effect and change the actors material to the damaged material
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	MeshComp->SetMaterial(0, DestroyedMaterial);
}

void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplosiveBarrel, IsDestroyed);
}