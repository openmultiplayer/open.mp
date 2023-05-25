# WARNING: Heavily modified fork of pawn-natives for internal use in open.mp. Use the upstream version!

# pawn-natives
Macros and templates for quickly defining PAWN (SA:MP mostly) natives and also exporting them for other plguins to use.

## Example.

### Old method:

(Example taken from the streamer plugin, because why not):

Header:

```cpp
	cell AMX_NATIVE_CALL IsValidDynamicCP(AMX *amx, cell *params);
```

Registration:

```cpp
	{ "IsValidDynamicCP", Natives::IsValidDynamicCP },
```

Implementation:

```cpp
cell AMX_NATIVE_CALL Natives::IsValidDynamicCP(AMX *amx, cell *params)
{
	CHECK_PARAMS(1, "IsValidDynamicCP");
	boost::unordered_map<int, Item::SharedCheckpoint>::iterator c = core->getData()->checkpoints.find(static_cast<int>(params[1]));
	if (c != core->getData()->checkpoints.end())
	{
		return 1;
	}
	return 0;
}
```

### New method:

```cpp
PAWN_NATIVE(Natives, IsValidDynamicCP, bool(int id))
{
	boost::unordered_map<int, Item::SharedCheckpoint>::iterator c = core->getData()->checkpoints.find(id);
	return (c != core->getData()->checkpoints.end());
}
```

### Advantages:

* No more messing about with registering natives.
* Correct types - no need to deal with `params`.
* Your natives are exported for other plugins to call directly (instead of going through an AMX).

## Macros

### PAWN_NATIVE

This will declare a static native and register it.  If you want the function available to the rest of your code in other files, you need to perform declaration and definition separately - just like prototyping any other normal C function:

Prototype (declaration):

```cpp
PAWN_NATIVE_DECL(Natives, IsValidDynamicCP, bool(int id));
```

Defintion:

```cpp
PAWN_NATIVE_DEFN(Natives, IsValidDynamicCP, bool(int id))
{
	boost::unordered_map<int, Item::SharedCheckpoint>::iterator c = core->getData()->checkpoints.find(id);
	return (c != core->getData()->checkpoints.end())
}
```

### PAWN_NATIVE_DECL

See above.

### PAWN_NATIVE_DEFN

See above.

### PAWN_NATIVE_DECLARE

Synonym for PAWN_NATIVE_DECL.

### PAWN_NATIVE_DEFINE

Synonym for PAWN_NATIVE_DEFN.

### PAWN_HOOK

This is similar to `PAWN_NATIVE`, but hooks an existing native function instead of creating an entirely new one.  It again exports your new version for calling directly (note that this may bypass other hooks on the same function):

```cpp
PAWN_HOOK(my_namespace, SetPlayerPos, bool(int playerid, float x, float y, float z))
{
	logprintf("my_namespace::SetPlayerPos called");
	return SetPlayerPos(playerid, x, y, z),
}
```

While this function is being run, the hook is disabled so that calling the original does not get stuck in an infinite loop.

For prototyping, there are also equivalent macros:

```cpp
PAWN_HOOK_DECL
PAWN_HOOK_DEFN
PAWN_HOOK_DECLARE
PAWN_HOOK_DEFINE
```

If you are using sampgdk (which is required for hooks anyway) you may need to undefine the existing symbols (unless you are not using the C++ wrappers):

```cpp
#undef SetPlayerPos
PAWN_HOOK(my_namespace, SetPlayerPos, bool(int playerid, float x, float y, float z))
{
	logprintf("my_namespace::SetPlayerPos called");
	return SetPlayerPos(playerid, x, y, z),
}
```

### PAWN_IMPORT

If you want to use the functions from another plugin in your plugin, simply add:

```cpp
PAWN_IMPORT(Natives, IsValidDynamicCP, bool(int id));
```

This is the same whether the original was a hook or a new native - so you as a user don't need to worry about which it is or if it is going change (that was a nice string of two-letter words, you don't see that very often).  Calling it then becomes as simple as:

```cpp
// You can also have `using namespace Natives;` if you want.
bool exists = Natives::IsValidDynamicCP(42);
```

## Use

### Inclusion

To use just the native declarations is easiest - there are no additional dependencies.  Just include the following to any file in which you wish to declare or use this new style of native:

```cpp
#include <pawn-natives/NativeFunc>
```

To use hooks, the inclusion is mostly the same:

```cpp
#include <pawn-natives/NativeHook>
```

However note that this include requires you to have [sampgdk](https://github.com/Zeex/sampgdk) - for accessing the original versions of natives, and [subhook](https://github.com/Zeex/subhook) - for installing the hooks at an assembly level.  These are assumed to be includable as `<subhook/file>` and `<sampgdk/file>`.  Again - if you only want to declare natives and not hooks, you do not need these two dependencies.

To import natives from another plugin, use this instead:

```cpp
#include <pawn-natives/NativeImport>
```

### Initialisation

Like most things, this does require initialisation.  Fortunately this is quite simple.  In one file (probably your main file) add:

```cpp
#include <pawn-natives/NativesMain>
```

That will provide storage space for required objects and variables.  It is important that this comes AFTER `NativeFunc` and `NativeHook` if you want them - it only includes objects for the parts it knows you want.

This is good:

```cpp
#include <pawn-natives/NativeFunc>
#include <pawn-natives/NativeHook>
#include <pawn-natives/NativesMain>
```

These are bad:

```cpp
#include <pawn-natives/NativeFunc>
#include <pawn-natives/NativesMain>
#include <pawn-natives/NativeHook>
```

```cpp
#include <pawn-natives/NativesMain>
#include <pawn-natives/NativeHook>
#include <pawn-natives/NativeFunc>
```

etc.

You also need to add a call to `pawn_natives::AmxLoad(amx);` in `AmxLoad`.  If all your natives use this method, return it:

```cpp
PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
	return pawn_natives::AmxLoad(amx);
}
```

If you are only importing natives, not declaring any, you don't need `NativesMain` or `pawn_natives::AmxLoad`.

### Calls

If you want to call a native or hook directly from your code, that is very easy:

```cpp
PAWN_NATIVE(my_namespace, Native2, bool(int id))
{
	return id == 42;
}

PAWN_NATIVE(my_namespace, Native1, bool(int id))
{
	return Native2(id);
}
```

If you want to call an original native, bypassing the hook, dereference it first:

```cpp
PAWN_HOOK(my_namespace, SetPlayerPos, bool(int playerid, float x, float y, float z))
{
	logprintf("my_namespace::SetPlayerPos called");
	return sampgdk::SetPlayerPos(playerid, x, y, z),
}

PAWN_NATIVE(my_namespace, SetPlayerPosAndAngle, bool(int playerid, float x, float y, float z, float a))
{
	(*SetPlayerPos)(playerid, x, y, z);
	return sampgdk::SetPlayerAngle(playerid, a);
}
```

That code will NOT print `my_namespace::SetPlayerPos called` because we bypassed the hook.  This will:

```cpp
PAWN_HOOK(my_namespace, SetPlayerPos, bool(int playerid, float x, float y, float z))
{
	logprintf("my_namespace::SetPlayerPos called");
	return sampgdk::SetPlayerPos(playerid, x, y, z),
}

PAWN_NATIVE(my_namespace, SetPlayerPosAndAngle, bool(int playerid, float x, float y, float z, float a))
{
	SetPlayerPos(playerid, x, y, z);
	return sampgdk::SetPlayerAngle(playerid, a);
}
```

Importantly, so will this - proving that hooks work even if you just call the original AMX functions from plugins:

```cpp
PAWN_HOOK(my_namespace, SetPlayerPos, bool(int playerid, float x, float y, float z))
{
	logprintf("my_namespace::SetPlayerPos called");
	return sampgdk::SetPlayerPos(playerid, x, y, z),
}

PAWN_NATIVE(my_namespace, SetPlayerPosAndAngle, bool(int playerid, float x, float y, float z, float a))
{
	sampgdk::SetPlayerPos(playerid, x, y, z);
	return sampgdk::SetPlayerAngle(playerid, a);
}
```

You can deal with the namespaces however you like - `using` or not.  Note that `pawn_natives` is a separate namespace to the one specified in your declarations, it holds the functions used to initialise the system itself.

### Logging

You can add debugging to the system by defining macros first.  For example:

```cpp
#define LOG_NATIVE_ERROR(...)   logprintf("ERROR: " __VA_ARGS__)
#define LOG_NATIVE_WARNING(...) logprintf("WARNING: " __VA_ARGS__)
#define LOG_NATIVE_DEBUG(...)   logprintf("DEBUG: " __VA_ARGS__)
#define LOG_NATIVE_INFO(...)    logprintf("INFO: " __VA_ARGS__)

#include <pawn-natives/NativeFunc>
#include <pawn-natives/NativeHook>
```

Or using [samp-log-core](https://github.com/maddinat0r/samp-log-core) (note that you may need a recent branch to allow variable parameters in the calls):

```cpp
#define LOG_NATIVE_ERROR(...)   gMyLogger(LogLevel::ERROR, __VA_ARGS__)
#define LOG_NATIVE_WARNING(...) gMyLogger(LogLevel::WARNING, __VA_ARGS__)
#define LOG_NATIVE_DEBUG(...)   gMyLogger(LogLevel::DEBUG, __VA_ARGS__)
#define LOG_NATIVE_INFO(...)    gMyLogger(LogLevel::INFO, __VA_ARGS__)

#include <pawn-natives/NativeFunc>
#include <pawn-natives/NativeHook>
```

You will also need to do that each time you include one of the headers in to a new file, so I suggest wrapping the whole lot in a new include.

### Seamless Use

The best way to use this library is in combination with sampgdk WITHOUT C++ wrappers.  To do this, ensure the symbol `SAMPGDK_CPP_WRAPPERS` is not defined anywhere.  This means that instead of:

```cpp
namespace sampgdk {

inline bool IsValidVehicle(int vehicleid) {
  return sampgdk_IsValidVehicle(vehicleid);
}
}
```

You get:

```cpp
#undef  IsValidVehicle
#define IsValidVehicle sampgdk_IsValidVehicle
```

Why is that better?  Surely namespaces are good and the pre-processor is bad?  Normally yes, but with this you can do:

```cpp
#undef SetPlayerPos
PAWN_HOOK_DECL(my_namespace, SetPlayerPos, bool(int playerid, float x, float y, float z));

PAWN_HOOK_DEFN(my_namespace, SetPlayerPos, bool(int playerid, float x, float y, float z))
{
	logprintf("my_namespace::SetPlayerPos called");
	return SetPlayerPos(playerid, x, y, z),
}

using namespace my_namespace;
```

Then in other code you just call:

```cpp
SetPlayerPos(playerid, 10.0, 10.0, 1000.0);
```

And you don't need to know if there is a hook or not.  If there is one, because of the `#undef` that will call the hook function directly, instead of going through the AMX.  If there isn't one, that will call `sampgdk_SetPlayerPos`.  Were we to use the namespaces instead of the pre-processor, we would have to use `sampgdk::SetPlayerPos` or `my_namespace::SetPlayerPos` explicitly.  Using two `using namespace`s would introduce two identically named symbols in to the current scope and the compiler would complain, forcing you to distinguish with the prefix despite using `using`.

This makes your code more future-proof.  You don't need to worry if there ever will be a hook on that function in the future - just write it like that and the compiler will deal with changes.

## Testing

```
# Before you start
pip install PLY cidl
```

```
mkdir build
cmake .
cd build
cmake --build ..
```

```
# Optional
cmake --build .. --config Release
```

```
# If there's a problem
cmake .. -DMAIN_RUN=FALSE
cmake --build ..
```


