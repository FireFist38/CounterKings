#include "TestDummyHealthBar.h"
#include "Components/ProgressBar.h"
#include "TestDummy.h"
#include "AttributeComponent.h"

void UTestDummyHealthBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (OwningDummy && HealthBar)
	{
		UAttributeComponent* Attr = OwningDummy->GetAttributeComponent();
		float Percent = Attr->GetMaxHealth() > 0.f ? Attr->GetCurrentHealth() / Attr->GetMaxHealth() : 0.f;
		HealthBar->SetPercent(Percent);
	}
}
