// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwordsRoyaleWeapon.generated.h"

UCLASS()
class SWORDSROYALE_API ASwordsRoyaleWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Static Mesh used to provide a visual representation of the object.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* SkeletalMesh;

	//The damage type and damage that will be done by this projectile
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<class UDamageType> DamageType;

	//The damage dealt by this projectile.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float Damage;

	UPROPERTY(EditAnywhere, Category = "Collision")
	TEnumAsByte<ECollisionChannel> TraceChannelProperty = ECC_Pawn;

	// Sets default values for this actor's properties
	ASwordsRoyaleWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called by Tick to see i weapon collided
	void CheckWeponHit();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
