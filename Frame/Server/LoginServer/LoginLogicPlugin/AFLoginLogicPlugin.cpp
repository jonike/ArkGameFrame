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

#include "AFLoginLogicPlugin.h"
#include "AFCLoginLogicModule.h"

#ifdef ARK_DYNAMIC_PLUGIN

ARK_EXPORT void DllStartPlugin(AFIPluginManager* pm)
{
#if ARK_PLATFORM == PLATFORM_WIN
    SetConsoleTitle("LoginServer -- ArkGame");
#endif
    CREATE_PLUGIN(pm, AFLoginLogicPlugin)

};

ARK_EXPORT void DllStopPlugin(AFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, AFLoginLogicPlugin)
};

#endif
//////////////////////////////////////////////////////////////////////////

const int AFLoginLogicPlugin::GetPluginVersion()
{
    return 0;
}

const std::string AFLoginLogicPlugin::GetPluginName()
{
    return GET_CLASS_NAME(AFLoginLogicPlugin);
}

void AFLoginLogicPlugin::Install()
{

    REGISTER_MODULE(pPluginManager, AFILoginLogicModule, AFCLoginLogicModule)

}

void AFLoginLogicPlugin::Uninstall()
{
    UNREGISTER_MODULE(pPluginManager, AFILoginLogicModule, AFCLoginLogicModule)
}

