#include "EditorValidator_Projectiles.h"

#include "Misc/DataValidation.h"
#include "Projectiles/RogueProjectile.h"


bool UEditorValidator_Projectiles::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return InObject && InObject->IsA<UBlueprint>();
}


EDataValidationResult UEditorValidator_Projectiles::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	const UBlueprint* BP = Cast<UBlueprint>(InAsset);
	
	ARogueProjectile* Projectile = Cast<ARogueProjectile>(BP->GeneratedClass.GetDefaultObject());
	if (Projectile == nullptr)
	{
		return EDataValidationResult::NotValidated;
	}

	// Basic example, require this to be set to avoid infinite projectiles
	if (Projectile->InitialLifeSpan <= 0.0f)
	{
		FText Output = FText::Join(FText::FromString(" "), FText::FromName(Projectile->GetFName()),
			FText::FromString(TEXT("Projectile has no InitialLifespan span and may exist forever.")));
		Context.AddError(Output);
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}