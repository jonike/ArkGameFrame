/*
* This source file is part of ArkGameFrame
* For the latest info, see https://github.com/ArkGame
*
* Copyright (c) 2013-2017 ArkGame authors.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include "AFGameLogicPlugin.h"
#include "AFCGameServerModule.h"
#include "AFCSceneProcessModule.h"
#include "AFCPropertyModule.h"
#include "AFCLevelModule.h"
#include "AFCPropertyConfigModule.h"
#include "AFCAccountModule.h"

#ifdef ARK_DYNAMIC_PLUGIN

ARK_EXPORT void DllStartPlugin(AFIPluginManager* pm)
{
#if ARK_PLATFORM == PLATFORM_WIN
    SetConsoleTitle("GameServer -- ArkGame");
#endif // ARK_PLATFORM
    CREATE_PLUGIN(pm, AFGameLogicPlugin)

};

ARK_EXPORT void DllStopPlugin(AFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, AFGameLogicPlugin)
};

#endif
//////////////////////////////////////////////////////////////////////////

const int AFGameLogicPlugin::GetPluginVersion()
{
    return 0;
}

const std::string AFGameLogicPlugin::GetPluginName()
{
    return GET_CLASS_NAME(AFGameLogicPlugin)
}

void AFGameLogicPlugin::Install()
{
    REGISTER_MODULE(pPluginManager, AFIGameServerModule, AFCGameServerModule)
    REGISTER_MODULE(pPluginManager, AFISceneProcessModule, AFCSceneProcessModule)
    REGISTER_MODULE(pPluginManager, AFIPropertyModule, AFCPropertyModule)
    REGISTER_MODULE(pPluginManager, AFILevelModule, AFCLevelModule)
    REGISTER_MODULE(pPluginManager, AFIPropertyConfigModule, AFCPropertyConfigModule)
    REGISTER_MODULE(pPluginManager, AFIAccountModule, AFCAccountModule)
}

void AFGameLogicPlugin::Uninstall()
{
    UNREGISTER_MODULE(pPluginManager, AFIAccountModule, AFCAccountModule)
    UNREGISTER_MODULE(pPluginManager, AFIPropertyConfigModule, AFCPropertyConfigModule)
    UNREGISTER_MODULE(pPluginManager, AFILevelModule, AFCLevelModule)
    UNREGISTER_MODULE(pPluginManager, AFIPropertyModule, AFCPropertyModule)
    UNREGISTER_MODULE(pPluginManager, AFISceneProcessModule, AFCSceneProcessModule)
    UNREGISTER_MODULE(pPluginManager, AFIGameServerModule, AFCGameServerModule)
}

