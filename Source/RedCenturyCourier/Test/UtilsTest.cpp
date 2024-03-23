// Copyright...

#include "Containers/UnrealString.h"
#include "../Utility/Utils.h"
#include "Misc/AutomationTest.h"


#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilsTest, "RedCenturyCourier.Utility.Utils", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)


bool FUtilsTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Component"),
		FUtils::Component(FVector::UpVector * 800.f, FVector::DownVector * 1600.f),
		FVector::UpVector * 800.f);
	TestEqual(TEXT("Component"),
		FUtils::Component(FVector::UpVector * 800.f + FVector::RightVector * 1000.f, FVector::UpVector),
		FVector::UpVector * 800.f);
	
	TestEqual(TEXT("Apex"),
		FUtils::Apex(FVector::UpVector, FVector::DownVector),
		FVector::UpVector * .5f);
	TestEqual(TEXT("Apex"),
		FUtils::Apex(FVector::UpVector * 800.f, FVector::DownVector * 1600.f),
		FVector::UpVector * 200.f);
	TestEqual(TEXT("Apex"),
		FUtils::Apex(FVector(400.f, 200.f, 269.f), FVector::DownVector * 1470.f),
		FVector::UpVector * 24.61258503f);
	
	TestEqual(TEXT("LimitImpulse"),
		FUtils::LimitImpulse(200.f, 700.f, 1000.f),
		200.f);
	TestEqual(TEXT("LimitImpulse"),
		FUtils::LimitImpulse(200.f, 800.f, 1000.f),
		200.f);
	TestEqual(TEXT("LimitImpulse"),
		FUtils::LimitImpulse(200.f, 900.f, 1000.f),
		100.f);
	TestEqual(TEXT("LimitImpulse"),
		FUtils::LimitImpulse(200.f, 1000.f, 1000.f),
		0.f);
	TestEqual(TEXT("LimitImpulse"),
		FUtils::LimitImpulse(200.f, 1100.f, 1000.f),
		0.f);
	
	TestEqual(TEXT("Adjustment"),
		FUtils::Adjustment(0.f, 100.f, 1.f, 100.f),
		100.f);
	TestEqual(TEXT("Adjustment"),
		FUtils::Adjustment(0.f, 100.f, .1f, 100.f),
		10.f);
	TestEqual(TEXT("Adjustment"),
		FUtils::Adjustment(0.f, 100.f, 1.5f, 100.f),
		100.f);
	TestEqual(TEXT("Adjustment"),
		FUtils::Adjustment(50.f, 100.f, 1.f, 100.f),
		50.f);
	TestEqual(TEXT("Adjustment"),
		FUtils::Adjustment(150.f, 100.f, 1.f, 100.f),
		0.f);
	
	TestEqual(TEXT("Adjustment"),
		FUtils::Adjustment(0.f, -100.f, 1.f, -100.f),
		-100.f);
	TestEqual(TEXT("Adjustment"),
		FUtils::Adjustment(0.f, -100.f, .1f, -100.f),
		-10.f);
	TestEqual(TEXT("Adjustment"),
		FUtils::Adjustment(0.f, -100.f, 1.5f, -100.f),
		-100.f);
	TestEqual(TEXT("Adjustment"),
		FUtils::Adjustment(-50.f, -100.f, 1.f, -100.f),
		-50.f);
	TestEqual(TEXT("Adjustment"),
		FUtils::Adjustment(-150.f, -100.f, 1.f, -100.f),
		0.f);
	
	// TestEqual(TEXT("ProjectToPlane"),
	// 	FUtils::ProjectToPlane(FVector(100,0,0), FVector(-1,0,1)),
	// 	FVector(100,0,100));
	// TestEqual(TEXT("ProjectToPlane"),
	// 	FUtils::ProjectToPlane(FVector(0,100,0), FVector(0,-1,1)),
	// 	FVector(0, 100,100));
	
	TestEqual(TEXT("ClampedAngularFInterpTo"),
		FUtils::ClampedAngularFInterpTo(0.f, 0.f, 1.f, 1.f, 180.f),
		0.f);
	TestEqual(TEXT("ClampedAngularFInterpTo"),
		FUtils::ClampedAngularFInterpTo(0.f, 50.f, 1.f, 1.f, 180.f),
		50.f);
	TestEqual(TEXT("ClampedAngularFInterpTo"),
		FUtils::ClampedAngularFInterpTo(0.f, 50.f, .5f, 1.f, 180.f),
		25.f);
	TestEqual(TEXT("ClampedAngularFInterpTo"),
		FUtils::ClampedAngularFInterpTo(0.f, 50.f, .5f, 2.f, 180.f),
		50.f);
	TestEqual(TEXT("ClampedAngularFInterpTo"),
		FUtils::ClampedAngularFInterpTo(0.f, 50.f, 100.f, 100.f, 180.f),
		50.f);
	TestEqual(TEXT("ClampedAngularFInterpTo"),
		FUtils::ClampedAngularFInterpTo(0.f, -50.f, 100.f, 100.f, 180.f),
		-50.f);
	TestEqual(TEXT("ClampedAngularFInterpTo"),
		FUtils::ClampedAngularFInterpTo(0.f, 50.f, 100.f, 100.f, 80.f),
		40.f);
	TestEqual(TEXT("ClampedAngularFInterpTo"),
		FUtils::ClampedAngularFInterpTo(0.f, -50.f, 100.f, 100.f, 80.f),
		-40.f);
	
	TestEqual(TEXT("ClampedRInterpTo"),
		FUtils::ClampedRInterpTo(FRotator(0.f, 0.f, 0.f), FRotator(0.f, 0.f, 0.f), 1.f, 1.f, FRotator(180.f, 180.f, 180.f)),
		FRotator(0.f, 0.f, 0.f));
	TestEqual(TEXT("ClampedRInterpTo"),
		FUtils::ClampedRInterpTo(FRotator(0.f, 0.f, 0.f), FRotator(50.f, 50.f, 50.f), 1.f, 1.f, FRotator(180.f, 180.f, 180.f)),
		FRotator(50.f, 50.f, 50.f));
	TestEqual(TEXT("ClampedRInterpTo"),
		FUtils::ClampedRInterpTo(FRotator(0.f, 0.f, 0.f), FRotator(50.f, 50.f, 50.f), .5f, 1.f, FRotator(180.f, 180.f, 180.f)),
		FRotator(25.f, 25.f, 25.f));
	TestEqual(TEXT("ClampedRInterpTo"),
		FUtils::ClampedRInterpTo(FRotator(0.f, 0.f, 0.f), FRotator(50.f, 50.f, 50.f), .5f, 2.f, FRotator(180.f, 180.f, 180.f)),
		FRotator(50.f, 50.f, 50.f));
	TestEqual(TEXT("ClampedRInterpTo"),
		FUtils::ClampedRInterpTo(FRotator(0.f, 0.f, 0.f), FRotator(50.f, 50.f, 50.f), 100.f, 100.f, FRotator(180.f, 180.f, 180.f)),
		FRotator(50.f, 50.f, 50.f));
	TestEqual(TEXT("ClampedRInterpTo"),
		FUtils::ClampedRInterpTo(FRotator(0.f, 0.f, 0.f), FRotator(-50.f, -50.f, -50.f), 100.f, 100.f, FRotator(180.f, 180.f, 180.f)),
		FRotator(-50.f, -50.f, -50.f));
	TestEqual(TEXT("ClampedRInterpTo"),
		FUtils::ClampedRInterpTo(FRotator(0.f, 0.f, 0.f), FRotator(50.f, 50.f, 50.f), 100.f, 100.f, FRotator(80.f, 80.f, 80.f)),
		FRotator(40.f, 40.f, 40.f));
	TestEqual(TEXT("ClampedRInterpTo"),
		FUtils::ClampedRInterpTo(FRotator(0.f, 0.f, 0.f), FRotator(-50.f, -50.f, -50.f), 100.f, 100.f, FRotator(80.f, 80.f, 80.f)),
		FRotator(-40.f, -40.f, -40.f));
	
	TestEqual(TEXT("ClampedRInterpTo"),
		FUtils::ClampedRInterpTo(FRotator(30.f, 30.f, 0.f), FRotator(40.f, -50.f, 0.f), .5f, 1.f, FRotator(150.f, 150.f, 180.f)),
		FRotator(35.f, -10.f, 0.f));
	TestEqual(TEXT("ClampedRInterpTo"),
		FUtils::ClampedRInterpTo(FRotator(30.f, 30.f, 0.f), FRotator(85.f, -50.f, 0.f), .5f, 1.f, FRotator(150.f, 150.f, 180.f)),
		FRotator(52.5f, -10.f, 0.f));
	TestEqual(TEXT("ClampedRInterpTo"),
		FUtils::ClampedRInterpTo(FRotator(30.f, 30.f, 0.f), FRotator(40.f, -180.f, 0.f), .5f, 1.f, FRotator(150.f, 150.f, 180.f)),
		FRotator(35.f, -22.5f, 0.f));
	TestEqual(TEXT("ClampedRInterpTo"),
		FUtils::ClampedRInterpTo(FRotator(30.f, 30.f, 0.f), FRotator(85.f, -180.f, 0.f), .5f, 1.f, FRotator(150.f, 150.f, 180.f)),
		FRotator(52.5f, -22.5f, 0.f));

	return true;
}


#undef TestSomething


#endif //WITH_DEV_AUTOMATION_TESTS
