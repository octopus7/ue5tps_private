#include "Modules/ModuleManager.h"
#include "CoverLineComponentVisualizer.h"
#include "Cover/Components/CoverLineComponent.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

class FCoverEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        if (GUnrealEd)
        {
            Visualizer = MakeShared<FCoverLineComponentVisualizer>();
            GUnrealEd->RegisterComponentVisualizer(UCoverLineComponent::StaticClass()->GetFName(), Visualizer);
        }
    }

    virtual void ShutdownModule() override
    {
        if (GUnrealEd)
        {
            GUnrealEd->UnregisterComponentVisualizer(UCoverLineComponent::StaticClass()->GetFName());
        }
        Visualizer.Reset();
    }

private:
    TSharedPtr<FComponentVisualizer> Visualizer;
};

IMPLEMENT_MODULE(FCoverEditorModule, CoverEditor)
