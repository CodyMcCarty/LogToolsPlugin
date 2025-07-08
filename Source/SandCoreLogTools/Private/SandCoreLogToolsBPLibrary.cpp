// Copyright Cody McCarty. All Rights Reserved.

#include "SandCoreLogToolsBPLibrary.h"

DEFINE_LOG_CATEGORY(LogSandGame);

FString USandCoreLogToolsBPLibrary::GetCallerContext(const UObject* WorldContextObject, const FString& Message, const TCHAR* Function)
{
	FString Result = FString::Printf(TEXT("[%s] | \"%s\"\t | %s"),
		*BuildPieRole(WorldContextObject),
		*Message,
		*BuildStackInfoWithLabel(WorldContextObject, Function));
	return Result;
}

FString USandCoreLogToolsBPLibrary::BuildPieRole(const UObject* WorldContextObject)
{
	FString ResultPieContextStr = TEXT("Invalid");

#if WITH_EDITOR

	if (!GIsEditor)
	{
		return ResultPieContextStr;
	}

	if (WorldContextObject == nullptr)
	{
		ResultPieContextStr = TEXT("Not in a play world");
		return ResultPieContextStr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (World == nullptr)
	{
		ResultPieContextStr = TEXT("(World Being Created)");
	}
	else if (World->WorldType == EWorldType::Editor)
	{
		ResultPieContextStr = TEXT("Editor");
	}
	else if (World->IsGameWorld())
	{
		const FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(World);

		switch (World->GetNetMode())
		{
		case NM_Standalone:
			ResultPieContextStr = TEXT("Standalone");
			break;
		case NM_ListenServer:
			ResultPieContextStr = TEXT("Server L");
			break;
		case NM_DedicatedServer:
			ResultPieContextStr = TEXT("Server D");
			break;
		case NM_Client:
			{
				//~ use either UE::GetPlayInEditorID() or GPlayInEditorID depending on the UE version.
				const int32 PieNum = WorldContext ? WorldContext->PIEInstance : UE::GetPlayInEditorID();
				ResultPieContextStr = FString::Printf(TEXT("Client %d"), PieNum);
			}
			break;
		default:
			unimplemented();
		};

		if (WorldContext && !WorldContext->CustomDescription.IsEmpty())
		{
			ResultPieContextStr += TEXT(" ") + WorldContext->CustomDescription;
		}
	}

#endif

	return ResultPieContextStr;
}

FString USandCoreLogToolsBPLibrary::BuildStackInfoWithLabel(const UObject* WorldContextObject, const TCHAR* Function)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) || USE_LOGGING_IN_SHIPPING
	/* Use this? if (GEditor)
	{
		if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEditor))
		{
			TOptional<FPlayInEditorSessionInfo> SessionInfo = EditorEngine->GetPlayInEditorSessionInfo();
			double PlayRequestStartTime = SessionInfo->PlayRequestStartTime;
			// ... */

	FStringBuilderBase Result; // todo reserve
	Result.Append(Function);

	Result.Append(TEXT(" | "));
	if (!WorldContextObject)
	{
		Result.Append(TEXT("Label Unavailable"));
	}
	else
	{
		// todo: spawned in vs placed with _C? update code and docs
		if (const AActor* Actor = Cast<const AActor>(WorldContextObject))
		{
			Result.Append(Actor->GetActorNameOrLabel());
		}
		else if (const AActor* TypedOuter = WorldContextObject->GetTypedOuter<AActor>(); ensure(TypedOuter))
		{
			Result.Append(TypedOuter->GetActorNameOrLabel());
		}
		else
		{
			Result.Append(WorldContextObject->GetName());
		}
	}

	Result.Append(TEXT(" | "));
	if (FBlueprintContextTracker::Get().GetCurrentScriptStack().IsEmpty())
	{
		Result.Append(TEXT("EmptyBPStack"));
	}
	else
	{
		const TArrayView<const FFrame* const> ScriptStack = FBlueprintContextTracker::Get().GetCurrentScriptStack();
		//~ NOTE: ScriptStack.Last()->Node->GetPackage()->GetFName(); //"/Game/BP_MyActor"
		//~ NOTE: WorldContextObject->GetClass()->GetFName(); //"BP_MyActor_C"
		if (!ScriptStack.Last()->Node->GetName().Contains(TEXT("ExecuteUbergraph")))
		{
			Result << ScriptStack.Last()->Node->GetPackage()->GetFName() << TEXT("..") << ScriptStack.Last()->Node->GetName();
		}
		else if (ScriptStack.Num() >= 2 && !ScriptStack[ScriptStack.Num() - 2]->Node->GetName().Contains(TEXT("ExecuteUbergraph")))
		{
			Result << ScriptStack.Last()->Node->GetPackage()->GetFName() << TEXT("..") << ScriptStack[ScriptStack.Num() - 2]->Node->GetName();
		}
		else
		{
			Result.Append(ScriptStack.Last()->Node->GetName());
		}
	}

	return FString(Result.ToString());

#else
	return FString();
#endif
}

void USandCoreLogToolsBPLibrary::SandCoreLogGame(const UObject* WorldContextObject, ESandCoreLogVerbosity Verbosity/*Log*/, const FText Message/*Hello*/)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) || USE_LOGGING_IN_SHIPPING //~ Do not Print in Shipping or Test unless explicitly enabled.
	// todo: test with USE_LOGGING_IN_SHIPPING
	FStringBuilderBase Result;
	Result << TEXT("[") << BuildPieRole(WorldContextObject) << TEXT("]");

	Result << TEXT(" | \"") << Message.ToString() << TEXT("\"\t");

	Result.Append(TEXT(" | "));
	if (FBlueprintContextTracker::Get().GetCurrentScriptStack().IsEmpty())
	{
		if (WorldContextObject)
		{
			Result.Append(WorldContextObject->GetClass()->GetName());
		}
		else
		{
			Result.Append(TEXT("Label Unavailable"));
		}
	}
	else
	{
		const TArrayView<const FFrame* const> ScriptStack = FBlueprintContextTracker::Get().GetCurrentScriptStack();
		if (!ScriptStack.Last()->Node->GetName().Contains(TEXT("ExecuteUbergraph")))
		{
			Result << ScriptStack.Last()->Node->GetPackage()->GetFName() << TEXT("..") << ScriptStack.Last()->Node->GetName();
		}
		else if (ScriptStack.Num() >= 2 && !ScriptStack[ScriptStack.Num() - 2]->Node->GetName().Contains(TEXT("ExecuteUbergraph")))
		{
			Result << ScriptStack.Last()->Node->GetPackage()->GetFName() << TEXT("..") << ScriptStack[ScriptStack.Num() - 2]->Node->GetName();
		}
		else
		{
			Result.Append(ScriptStack.Last()->Node->GetName());
		}
	}

	Result.Append(TEXT(" | "));
	if (!WorldContextObject)
	{
		Result.Append(TEXT("Label Unavailable"));
	}
	else
	{
		// todo: spawned in vs placed with _C? update code and docs
		if (const AActor* Actor = Cast<const AActor>(WorldContextObject))
		{
			Result.Append(Actor->GetActorNameOrLabel());
		}
		else if (const AActor* TypedOuter = WorldContextObject->GetTypedOuter<AActor>(); ensure(TypedOuter))
		{
			Result.Append(TypedOuter->GetActorNameOrLabel());
		}
		else
		{
			Result.Append(WorldContextObject->GetName());
		}
	}

	Result.Append(TEXT(" | "));
	FString LastCppCall(TEXT("No Useful Symbol"));
	{
		constexpr int32 MaxDepth = 32;
		uint64 BackTrace[MaxDepth] = {0};
		const int32 Depth = FPlatformStackWalk::CaptureStackBackTrace(BackTrace, UE_ARRAY_COUNT(BackTrace));
		static const TArray<FString> ModuleBlackList =
		{
			TEXT("UnrealEditor-CoreUObject.dll"),
			TEXT("UnrealEditor-Core.dll"),
			TEXT("SandCoreLogTools"),
		};
		for (int32 i = 0; i < Depth; ++i)
		{
			FProgramCounterSymbolInfo SymbolInfo{};
			FPlatformStackWalk::ProgramCounterToSymbolInfo(BackTrace[i], SymbolInfo);
			FString ModuleName = ANSI_TO_TCHAR(SymbolInfo.ModuleName);
			bool bSkip = false;
			for (const FString& SkipModule : ModuleBlackList)
			{
				if (ModuleName.Contains(SkipModule))
				{
					bSkip = true;
					break;
				}
			}
			if (!bSkip)
			{
				LastCppCall = ANSI_TO_TCHAR(SymbolInfo.FunctionName);
				if (!LastCppCall.Contains(TEXT("ProcessEvent")))
				{
					break;
				}
			}
		}
	}
	Result.Append(LastCppCall);

	const FString FinalLogString = Result.ToString();

	switch (Verbosity)
	{
	case ESandCoreLogVerbosity::Fatal:
		UE_LOG(LogSandGame, Fatal, TEXT("%s"), *FinalLogString);
		break;
	case ESandCoreLogVerbosity::Error:
		{
			const uint64 Key = GetTypeHash(GetNameSafe(WorldContextObject));
			GEngine->AddOnScreenDebugMessage(Key, 10.f, FColor::Red, FinalLogString);
			UE_LOG(LogSandGame, Error, TEXT("%s"), *FinalLogString);
		}
		break;
	case ESandCoreLogVerbosity::Warning:
		{
			const uint64 Key = GetTypeHash(GetNameSafe(WorldContextObject));
			GEngine->AddOnScreenDebugMessage(Key, 10.f, FColor::Orange, FinalLogString);
			UE_LOG(LogSandGame, Warning, TEXT("%s"), *FinalLogString);
		}
		break;
	case ESandCoreLogVerbosity::Display:
		UE_LOG(LogSandGame, Display, TEXT("%s"), *FinalLogString);
		break;
	case ESandCoreLogVerbosity::Log:
		UE_LOG(LogSandGame, Log, TEXT("%s"), *FinalLogString);
		break;
	case ESandCoreLogVerbosity::Verbose:
		UE_LOG(LogSandGame, Verbose, TEXT("%s"), *FinalLogString);
		break;
	case ESandCoreLogVerbosity::VeryVerbose:
		UE_LOG(LogSandGame, VeryVerbose, TEXT("%s"), *FinalLogString);
		break;
	}
#endif
}
