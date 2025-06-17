#include "EditorValidator_Projectiles.h"

#include "Misc/DataValidation.h"
#include "Projectiles/RogueProjectile.h"


bool UEditorValidator_Projectiles::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	if (const UBlueprint* BP = Cast<UBlueprint>(InObject))
	{
		return BP->GeneratedClass.GetDefaultObject()->IsA<ARogueProjectile>();
	}

	return false;
}


EDataValidationResult UEditorValidator_Projectiles::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	const UBlueprint* BP = CastChecked<UBlueprint>(InAsset);
	check(BP);
	
	ARogueProjectile* Projectile = Cast<ARogueProjectile>(BP->GeneratedClass.GetDefaultObject());
	check(Projectile);

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