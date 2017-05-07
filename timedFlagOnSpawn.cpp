/*
    Copyright (C) 2017 Vladimir "allejo" Jimenez

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the “Software”), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include <memory>

#include "bzfsAPI.h"
#include "bztoolkit/bzToolkitAPI.h"

// Define plug-in name
const std::string PLUGIN_NAME = "Timed Flag On Spawn";

// Define plug-in version numbering
const int MAJOR = 1;
const int MINOR = 0;
const int REV = 1;
const int BUILD = 6;

const int VERBOSE_LVL = 0;

class timedFlagOnSpawn : public bz_Plugin
{
public:
    virtual const char* Name ();
    virtual void Init (const char* config);
    virtual void Cleanup ();
    virtual void Event (bz_EventData* eventData);

    struct FlagDefinition
    {
        std::string flag;
        int delay;
    };

private:
    struct FlagStatus
    {
        int flagID;
        int delay;
        double givenAt;
        bool needsToBeTaken;

        FlagStatus() :
            flagID(-1),
            delay(0),
            givenAt(0),
            needsToBeTaken(false) {}
    };

    virtual void checkPlayerFlag (int playerID);
    virtual void parseFlagListDefinition (const char* configuration);
    virtual void tryParseFlagDefinition (std::string definition);
    virtual FlagDefinition parseFlagDefinition (const char* definition);

    std::vector<FlagDefinition> flagDefinitions;
    std::map<int, FlagStatus> flagsGiven;
};

BZ_PLUGIN(timedFlagOnSpawn)

const char* timedFlagOnSpawn::Name ()
{
    static std::string pluginName;

    if (pluginName.empty())
        pluginName = bztk_pluginName(PLUGIN_NAME, MAJOR, MINOR, REV, BUILD);

    return pluginName.c_str();
}

void timedFlagOnSpawn::Init (const char* config)
{
    Register(bz_eFlagDroppedEvent);
    Register(bz_ePlayerJoinEvent);
    Register(bz_ePlayerSpawnEvent);
    Register(bz_ePlayerUpdateEvent);

    parseFlagListDefinition(config);

    if (flagDefinitions.empty())
    {
        bz_debugMessage(0, "WARNING :: timedFlagOnSpawn :: You have not loaded any flag definitions");
    }
}

void timedFlagOnSpawn::Cleanup ()
{
    Flush();
}

void timedFlagOnSpawn::Event (bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
        case bz_eFlagDroppedEvent:
        {
            bz_FlagDroppedEventData_V1 *dropData = (bz_FlagDroppedEventData_V1*)eventData;

            flagsGiven[dropData->playerID].needsToBeTaken = false;
        }
        break;

        case bz_ePlayerJoinEvent:
        {
            bz_PlayerJoinPartEventData_V1 *joinData = (bz_PlayerJoinPartEventData_V1*)eventData;

            flagsGiven[joinData->playerID] = FlagStatus();
        }
        break;

        case bz_ePlayerSpawnEvent:
        {
            bz_PlayerSpawnEventData_V1 *spawnData = (bz_PlayerSpawnEventData_V1*)eventData;

            if (flagDefinitions.empty())
            {
                return;
            }

            FlagDefinition flag = *bztk_select_randomly(flagDefinitions.begin(), flagDefinitions.end());

            bz_givePlayerFlag(spawnData->playerID, flag.flag.c_str(), true);

            int flagID = bz_getPlayerFlagID(spawnData->playerID);

            flagsGiven[spawnData->playerID].needsToBeTaken = (flag.delay > 0);
            flagsGiven[spawnData->playerID].givenAt = bz_getCurrentTime();
            flagsGiven[spawnData->playerID].flagID = flagID;
            flagsGiven[spawnData->playerID].delay = flag.delay;

            bz_debugMessagef(VERBOSE_LVL, "DEBUG :: timedFlagOnSpawn :: player %d was given the %s flag (%d) %s",
                             spawnData->playerID, flag.flag.c_str(), flagID,
                             (flag.delay > 0) ? bz_format("for %d seconds", flag.delay) : "indefinitely");
        }
        break;

        case bz_ePlayerUpdateEvent:
        {
            bz_PlayerUpdateEventData_V1 *updateData = (bz_PlayerUpdateEventData_V1*)eventData;

            if (updateData->state.status == eAlive)
            {
                checkPlayerFlag(updateData->playerID);
            }
        }
        break;

        default: break;
    }
}

void timedFlagOnSpawn::checkPlayerFlag (int playerID)
{
    FlagStatus &status = flagsGiven[playerID];

    if (!status.needsToBeTaken)
    {
        return;
    }

    bool timesUp = (status.givenAt + status.delay < bz_getCurrentTime());
    bool sameFlag = (status.flagID == bz_getPlayerFlagID(playerID));

    if (timesUp && sameFlag)
    {
        bz_removePlayerFlag(playerID);

        status.needsToBeTaken = false;

        bz_debugMessagef(VERBOSE_LVL, "DEBUG :: timedFlagOnSpawn :: player %d's flag was taken", playerID);
        bz_debugMessagef(VERBOSE_LVL, "DEBUG :: timedFlagOnSpawn ::     current time: %.0f", bz_getCurrentTime());
        bz_debugMessagef(VERBOSE_LVL, "DEBUG :: timedFlagOnSpawn ::     given at: %.0f", status.givenAt);
        bz_debugMessagef(VERBOSE_LVL, "DEBUG :: timedFlagOnSpawn ::     delay: %d", status.delay);
    }
}

// Parse the flag configuration given at load time. The flag list definition is separated by semi-colons.
void timedFlagOnSpawn::parseFlagListDefinition (const char* configuration)
{
    std::string workspace = configuration;

    if (workspace.find(";") == std::string::npos)
    {
        tryParseFlagDefinition(configuration);
    }
    else
    {
        bz_APIStringList flagDefList;
        flagDefList.tokenize(configuration, ";");

        for (unsigned int i = 0; i < flagDefList.size(); i++)
        {
            tryParseFlagDefinition(flagDefList[i]);
        }
    }
}

// Safely try to load flag definitions into our vector
void timedFlagOnSpawn::tryParseFlagDefinition (std::string definition)
{
    try
    {
        FlagDefinition def = parseFlagDefinition(definition.c_str());
        flagDefinitions.push_back(def);
    }
    catch (const std::runtime_error &e)
    {
        bz_debugMessagef(0, "ERROR :: timedFlagOnSpawn :: A flag definition syntax is as follows: <flag abbr>=<time in seconds>");
        bz_debugMessagef(0, "ERROR :: timedFlagOnSpawn ::     error found in: %s", definition.c_str());
    }
    catch (const std::exception &e)
    {
        bz_debugMessagef(0, "ERROR :: timedFlagOnSpawn :: The time delay for flags being take must be an integer");
        bz_debugMessagef(0, "ERROR :: timedFlagOnSpawn ::     got the following error: %s", e.what());
    }
}

// Parse an individual flag definition. This should respect the following syntax:
//     syntax:  <flag>=<time>
//     example: WG=15
timedFlagOnSpawn::FlagDefinition timedFlagOnSpawn::parseFlagDefinition (const char* definition)
{
    std::string workspace = definition;
    bz_APIStringList def;
    def.tokenize(definition, "=");

    if (workspace.find("=") == std::string::npos || def.size() != 2)
    {
        throw std::runtime_error("Invalid flag definition");
    }

    FlagDefinition flagDef;
    flagDef.flag = def[0];
    flagDef.delay = std::stoi(def[1]);

    bz_debugMessagef(VERBOSE_LVL, "DEBUG :: timedFlagOnSpawn :: Flag Definition parsed as %s flag will be given for %d seconds", flagDef.flag.c_str(), flagDef.delay);

    return flagDef;
}
