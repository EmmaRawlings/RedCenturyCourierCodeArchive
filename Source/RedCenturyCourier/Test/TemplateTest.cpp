// Copyright...

#include "CoreTypes.h"
#include "Containers/UnrealString.h"
#include "Misc/AutomationTest.h"
// #include "Somewhere/Something.h"


#if WITH_DEV_AUTOMATION_TESTS

#define TestSomething(Desc, A, B) if ((A) != (B)) AddError(FString::Printf(TEXT("%s - A=%d B=%d"), Desc, (A), (B)));

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTemplateTest, "RedCenturyCourier.Somewhere.Something", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)


bool FTemplateTest::RunTest(const FString& Parameters)
{
	// const FSomething Testee = FSomething();
	//
	// TestSomething(TEXT("Testing something"), Testee.actual(), expected);
	// TestEqual(TEXT("Testing something equals"), Testee.actuallyEqual(), expectedEqual);

	return true;
}


#undef TestSomething


#endif //WITH_DEV_AUTOMATION_TESTS
