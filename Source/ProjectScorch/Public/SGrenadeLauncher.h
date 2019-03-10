// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeaponBase.h"
#include "SGrenadeLauncher.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTSCORCH_API ASGrenadeLauncher : public ASWeaponBase
{
	GENERATED_BODY()
	
	
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AActor>ProjectileClass;

	virtual void Fire() override;


public:


};
