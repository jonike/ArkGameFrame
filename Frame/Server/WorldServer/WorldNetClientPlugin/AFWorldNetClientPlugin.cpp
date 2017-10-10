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

#include "AFWorldNetClientPlugin.h"
#include "AFCWorldToMasterModule.h"

#ifdef ARK_DYNAMIC_PLUGIN

ARK_EXPORT void DllStartPlugin(AFIPluginManager* pm)
{
    CREATE_PLUGIN(pm, AFWorldNetClientPlugin)
}

ARK_EXPORT void DllStopPlugin(AFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, AFWorldNetClientPlugin)
}

#endif
//////////////////////////////////////////////////////////////////////////

const int AFWorldNetClientPlugin::GetPluginVersion()
{
    return 0;
}

const std::string AFWorldNetClientPlugin::GetPluginName()
{
    return GET_CLASS_NAME(AFWorldNetClientPlugin)
}

void AFWorldNetClientPlugin::Install()
{
    REGISTER_MODULE(pPluginManager, AFIWorldToMasterModule, AFCWorldToMasterModule)
}

void AFWorldNetClientPlugin::Uninstall()
{
    UNREGISTER_MODULE(pPluginManager, AFIWorldToMasterModule, AFCWorldToMasterModule)
}