#include "Modules/ModuleManager.h"

class FCoverModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FCoverModule, Cover)
