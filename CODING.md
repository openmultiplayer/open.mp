# Pass-by-value vs pass-by-reference

### Always try to pass by reference unless you're passing:
* Basic types (int, char, etc.)
* StringView/Span
* Vector2/Vector3/Vector4/GTAQuat

### ALWAYS pass these by const reference in the SDK headers:
* Any ABI-stable dynamic memory allocating objects (StaticArray, FlatHashMap, FlatHashSet, etc.)
* StaticString (though you should avoid passing it directly and pass StringView instead)

### NEVER use these in the SDK headers:
* Any ABI-unstable objects from STL (String, DynamicArray, etc.)

# PAWN script parameters

### How parameters are mapped to PAWN natives:
| API_SCRIPT | PAWN |
| ---------- | ---- |
| int | value | 
| float | Float:f |
| Vector2 | Float:x, Float:y |
| Vector3 | Float:x, Float:y, Float:z |
| Vector4 | Float:x, Float:y, Float:z, Float:w |
| GTAQuat | Float:w, Float:x, Float:y, Float:z |
| String  | value[], len |
| Pool entries (IPlayer, IVehicle, etc.) | poolid |
| Player pool entries (IPlayerObject, IPlayerTextLabel, etc.) | playerid, ..., poolid |

### Parameter attributes:
| Attribute | Transmit | Types |
| --------- | -------- | ----- |
| no specifier | read-only | int, float, Vector2, Vector3, Vector4, GTAQuat |
| const& | read-only | String |
| & | read-write | int, float, Vector2, Vector3, Vector4, GTAQuat, String |
| & | read-only | Pool entries, Player pool entries |
| * | read-only, optional | Pool entries, Player pool entries |
