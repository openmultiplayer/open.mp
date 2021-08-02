# Pass-by-value vs pass-by-reference

### Always try to pass by reference unless you're passing:
* Basic types (int, char, etc.)
* StringView
* Vector2/Vector3/Vector4/GTAQuat

### ALWAYS pass these by const reference in the SDK headers:
* Any dynamic memory allocating objects (StaticString, FlatHashMap, FlatHashSet, etc.)

### NEVER use these in the SDK headers:
* Any ABI-unstable objects from STL (String, DynamicArray, etc.)
