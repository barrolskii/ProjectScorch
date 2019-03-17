// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class UStaticMeshComponent;
class USHealthComponent;
class URadialForceComponent;
class UParticleSystem;

UCLASS()
class PROJECTSCORCH_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	

	ASExplosiveBarrel();

protected:

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent *MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent *RadialForceComp;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	USHealthComponent *HealthComp;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, 
						 const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	bool IsDestroyed;
	
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float ExplosionImpulse;
	
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem *ExplosionEffect;

	/* Material to swap to when the barrel has been destroyed */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UMaterialInterface *DestroyedMaterial;

};