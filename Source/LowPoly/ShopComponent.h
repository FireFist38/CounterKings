#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemBase.h"
#include "ShopComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopUpdated);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LOWPOLY_API UShopComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UShopComponent();

	UPROPERTY(BlueprintAssignable, Category = "CK|Shop")
	FOnShopUpdated OnShopUpdated;

protected:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "CK|Shop")
	TArray<TSubclassOf<AItemBase>> CurrentShopPool;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "CK|Shop")
    TArray<bool> LockedSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Shop")
	class UDataTable* ShopDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Shop")
	class ULootGenerationData* GenerationSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CK|Shop")
	int32 RerollCost = 5;

public:
	void GenerateShopPool(int32 Round, float Luck);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "CK|Shop")
	void Server_PurchaseItem(int32 SlotIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "CK|Shop")
	void Server_SellItem(int32 InventoryIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "CK|Shop")
	void Server_RerollShop();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "CK|Shop")
    void Server_ToggleLockSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "CK|Shop")
	TArray<TSubclassOf<AItemBase>> GetShopPool() const { return CurrentShopPool; }

    UFUNCTION(BlueprintCallable, Category = "CK|Shop")
    TArray<bool> GetLockedSlots() const { return LockedSlots; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
