#pragma once
/******************************************************
* - Originally written by Y_Less, taken from old open.mp code

*/

#include <Impl/events_impl.hpp>
#include <Server/Components/Actors/actors.hpp>
#include <Server/Components/Checkpoints/checkpoints.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Console/console.hpp>
#include <Server/Components/Databases/databases.hpp>
#include <Server/Components/Dialogs/dialogs.hpp>
#include <Server/Components/GangZones/gangzones.hpp>
#include <Server/Components/Menus/menus.hpp>
#include <Server/Components/Objects/objects.hpp>
#include <Server/Components/Pawn/pawn.hpp>
#include <Server/Components/Pickups/pickups.hpp>
#include <Server/Components/TextDraws/textdraws.hpp>
#include <Server/Components/TextLabels/textlabels.hpp>
#include <Server/Components/Timers/timers.hpp>
#include <Server/Components/Variables/variables.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <sdk.hpp>

#include <algorithm>
#include <map>
#include <memory>
#include <string>

#include "../PluginManager/PluginManager.hpp"
#include "../Script/Script.hpp"
#include "../Singleton.hpp"

using namespace Impl;

class PawnManager : public Singleton<PawnManager> {
public:
    FlatHashMap<String, std::unique_ptr<PawnScript>> scripts_;
    std::string entryScript = "";
    FlatHashMap<AMX*, PawnScript*> amxToScript_;
    ICore* core = nullptr;
    IConfig* config = nullptr;
    IPlayerPool* players = nullptr;
    IActorsComponent* actors = nullptr;
    ICheckpointsComponent* checkpoints = nullptr;
    IClassesComponent* classes = nullptr;
    IConsoleComponent* console = nullptr;
    IDatabasesComponent* databases = nullptr;
    IDialogsComponent* dialogs = nullptr;
    IGangZonesComponent* gangzones = nullptr;
    IMenusComponent* menus = nullptr;
    IObjectsComponent* objects = nullptr;
    IPickupsComponent* pickups = nullptr;
    ITextDrawsComponent* textdraws = nullptr;
    ITextLabelsComponent* textlabels = nullptr;
    ITimersComponent* timers = nullptr;
    IVariablesComponent* vars = nullptr;
    IVehiclesComponent* vehicles = nullptr;
    DefaultEventDispatcher<PawnEventHandler> eventDispatcher;
    PawnPluginManager pluginManager;
    int gamemodeIndex_ = 0;
    DynamicArray<String> gamemodes_;

    PawnManager();
    ~PawnManager();

    void printPawnLog(const std::string& type, const std::string& message)
    {
        core->printLn("[PAWN-LOG] %s: %s", type.c_str(), message.c_str());
    }

    void SetBasePath(std::string const& path);
    void SetScriptPath(std::string const& path);

    bool Load(std::string const& name, bool primary = false);
    bool Reload(std::string const& name);
    bool Unload(std::string const& name);
    bool Changemode(std::string const& name);

    template <typename... T>
    cell CallAllInSidesFirst(char const* name, DefaultReturnValue defaultRetValue, T... args)
    {
        cell ret = static_cast<cell>(defaultRetValue);

        PawnScript* first = nullptr;
        for (auto& cur : scripts_) {
            if (cur.first == entryScript) {
                first = cur.second.get();
            } else {
                ret = cur.second->Call(name, defaultRetValue, args...);
            }
        }

        if (first != nullptr) {
            ret = first->Call(name, defaultRetValue, args...);
        }

        return ret;
    }

    template <typename... T>
    cell CallAllInEntryFirst(char const* name, DefaultReturnValue defaultRetValue, T... args)
    {
        cell ret = static_cast<cell>(defaultRetValue);

        FlatHashMap<String, std::unique_ptr<PawnScript>>::const_iterator const& first = scripts_.find(entryScript);
        if (first != scripts_.end()) {
            ret = first->second->Call(name, defaultRetValue, args...);
        }

        for (auto& cur : scripts_) {
            if (cur.first == entryScript) {
                continue;
            } else {
                ret = cur.second->Call(name, defaultRetValue, args...);
            }
        }

        return ret;
    }

    template <typename... T>
    cell CallInSidesWhile0(char const* name, T... args)
    {
        cell
            ret
            = 0;

        for (auto& cur : scripts_) {
            if (cur.first == entryScript) {
                continue;
            } else {
                ret = cur.second->Call(name, DefaultReturnValue_False, args...);
                if (ret) {
                    break;
                }
            }
        }

        return ret;
    }

    template <typename... T>
    cell CallInSidesWhile1(char const* name, T... args)
    {
        cell
            ret
            = 1;

        for (auto& cur : scripts_) {
            if (cur.first == entryScript) {
                continue;
            } else {
                ret = cur.second->Call(name, DefaultReturnValue_True, args...);
                if (!ret) {
                    break;
                }
            }
        }

        return ret;
    }

    template <typename... T>
    cell CallInSides(char const* name, DefaultReturnValue defaultRetValue, T... args)
    {
        cell ret = static_cast<cell>(defaultRetValue);

        for (auto& cur : scripts_) {
            if (cur.first == entryScript) {
                continue;
            } else {
                ret = cur.second->Call(name, defaultRetValue, args...);
            }
        }

        return ret;
    }

    template <typename... T>
    cell CallInEntry(char const* name, DefaultReturnValue defaultRetValue, T... args)
    {
        cell ret = static_cast<cell>(defaultRetValue);

        FlatHashMap<String, std::unique_ptr<PawnScript>>::const_iterator const& first = scripts_.find(entryScript);
        if (first != scripts_.end()) {
            ret = first->second->Call(name, defaultRetValue, args...);
        }

        return ret;
    }

    template <typename... T>
    cell CallAll(char const* name, T... args)
    {
        cell
            ret
            = 0;
        for (auto& cur : scripts_) {
            ret = cur.second->Call(name, DefaultReturnValue_False, args...);
        }
        return ret;
    }

    template <typename... T>
    cell CallAll(std::string const& name, T... args)
    {
        cell
            ret
            = 0;
        for (auto& cur : scripts_) {
            ret = cur.second->Call(name, DefaultReturnValue_False, args...);
        }
        return ret;
    }

    template <typename... T>
    cell CallWhile0(char const* name, T... args)
    {
        cell
            ret
            = 0;
        for (auto& cur : scripts_) {
            ret = cur.second->Call(name, DefaultReturnValue_False, args...);
            if (ret)
                return ret;
        }
        return ret;
    }

    template <typename... T>
    cell CallWhile0(std::string const& name, T... args)
    {
        cell ret = static_cast<cell>(DefaultReturnValue_False);

        for (auto& cur : scripts_) {
            ret = cur.second->Call(name, DefaultReturnValue_False, args...);
            if (ret)
                return ret;
        }
        return ret;
    }

    template <typename... T>
    cell CallWhile1(char const* name, T... args)
    {
        cell ret = static_cast<cell>(DefaultReturnValue_True);

        for (auto& cur : scripts_) {
            ret = cur.second->Call(name, DefaultReturnValue_True, args...);
            if (!ret)
                return ret;
        }
        return ret;
    }

    template <typename... T>
    cell CallWhile1(std::string const& name, T... args)
    {
        cell ret = static_cast<cell>(DefaultReturnValue_True);

        for (auto& cur : scripts_) {
            ret = cur.second->Call(name, DefaultReturnValue_True, args...);
            if (!ret)
                return ret;
        }
        return ret;
    }

    AMX* AMXFromID(int id) const;
    int IDFromAMX(AMX*) const;

    bool OnServerCommand(const ConsoleCommandSenderData& sender, std::string const& cmd, std::string const& args);

private:
    std::string
        scriptPath_,
        basePath_;
    int
        id_
        = 0;

    void CheckNatives(PawnScript& script);
};
