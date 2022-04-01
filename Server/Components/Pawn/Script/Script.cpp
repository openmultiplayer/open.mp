/******************************************************
* - Originally written by Y_Less, taken from old open.mp code

*/

#define _Static_assert static_assert

#include <assert.h>
#include <stdarg.h>

#include "Script.hpp"

extern "C" {
int AMXEXPORT amx_ArgsInit(AMX* amx);
int AMXEXPORT amx_ArgsCleanup(AMX* amx);

int AMXEXPORT amx_ConsoleInit(AMX* amx);
int AMXEXPORT amx_ConsoleCleanup(AMX* amx);

int AMXEXPORT amx_CoreInit(AMX* amx);
int AMXEXPORT amx_CoreCleanup(AMX* amx);

int AMXEXPORT amx_FileInit(AMX* amx);
int AMXEXPORT amx_FileCleanup(AMX* amx);

int AMXEXPORT amx_StringInit(AMX* amx);
int AMXEXPORT amx_StringCleanup(AMX* amx);

int AMXEXPORT amx_TimeInit(AMX* amx);
int AMXEXPORT amx_TimeCleanup(AMX* amx);

int AMXEXPORT amx_FloatInit(AMX* amx);
int AMXEXPORT amx_FloatCleanup(AMX* amx);
}

/// A map of per-AMX caches
static FlatHashMap<AMX*, AMXCache*> cache;

PawnScript::PawnScript(int id, std::string const& path, ICore* core)
    : serverCore(core)
    , id_(id)
{
    int err = aux_LoadProgram(&amx_, const_cast<char*>(path.c_str()), nullptr);
    switch (err) {
    case AMX_ERR_NOTFOUND:
        serverCore->printLn("Could not find:\n\n\t %s %s", path.c_str(),
            R"(
                While attempting to load a PAWN gamemode, a file-not-found error was
                encountered.  This could be caused by many things:
                
                * The wrong filename was given.
                * The wrong gamemodes path was given.
                * The server was launched from a different directory, making relative paths relative to the wrong place (and thus wrong).
                * You didn't copy the file to the correct directory or server.
                * The compilation failed, leading to no output file.
                * `-l` or `-a` were used to compile, which output intermediate steps for inspecting, rather than a full script.
                * Anything else, really just check the file is at the path given.
            )");
        break;
    case AMX_ERR_NONE:
        loaded_ = true;
        break;
    default:
        serverCore->printLn("%s", aux_StrError(err));
        return;
    }

    if (loaded_) {
        amx_ArgsInit(&amx_);
        amx_CoreInit(&amx_);
        amx_FileInit(&amx_);
        amx_StringInit(&amx_);
        amx_TimeInit(&amx_);
        amx_FloatInit(&amx_);
        cache.emplace(std::make_pair<AMX*, AMXCache*>(&amx_, &cache_));
    } else {
        return;
    }
}

PawnScript::~PawnScript()
{
    if (loaded_) {
        amx_FloatCleanup(&amx_);
        amx_TimeCleanup(&amx_);
        amx_StringCleanup(&amx_);
        amx_FileCleanup(&amx_);
        amx_CoreCleanup(&amx_);
        amx_ArgsCleanup(&amx_);
        aux_FreeProgram(&amx_);
        cache.erase(&amx_);
    }
}

/*
void PawnScript::SubscribeAll()
{

}

void PawnScript::PrintError(int err)
{
    std::string pawnError = aux_StrError(num);
    switch (err)
    {
    case AMX_ERR_EXIT:         // forced exit
        throw PawnException(err)
            << R"(
This is triggered by the `exit` keyword in PAWN:

```pawn
    new
        a = 5,
        b = 2;
    // Will pass, because `5` is greater than `2`.
    if (a > b)
    {
        exit;
    }
```
)";
    case AMX_ERR_ASSERT:       // assertion failed
        throw PawnException(err)
            >> R"(
This is triggered by the `assert` keyword in PAWN, when compiling with
debugging enabled, and the condition is false:

```pawn
    #pragma option -d1
    new
        a = 5,
        b = 2;
    // Will fail, because `5` is greater than `2`.
    assert(a <= b);
```
)";
    case AMX_ERR_STACKERR:     // stack/heap collision
        throw PawnException(err)
            >> R"(
This generally happens when too many local variables are created; as a few very
large variables, lots of small ones, or too much recursion recreating the same
ones over and over again:

```pawn
    FuncA()
    {
        new
            // One large.
            largeLocal[1024],
            // Many small.
            a, b, c, d, e, f, g, h, i, j, k, l, m,
            n, o, p, q, r, s, t, u, v, w, x, y, z;
        // Recursion.
        FuncA();
    }
```

Note that none of these techniques are bad on their own, it is only when they
are used extensively or excessively that the memory requirements mount.  With
`-v` or `-d3` you can get a local memory report from the compiler:

```
    Header size:         1744 bytes
    Code size:          97808 bytes
    Data size:          35028 bytes
    Stack/heap size:   239360 bytes; estimated max. usage=138 cells (552 bytes)
    Total requirements:273940 bytes
```

It is the `Stack/heap size` and `estimated max. usage` statistics that are
important here.  Obviously the size should be greater than the usage.  The
compiler can also detect some overflows and will display the above message in
those cases.

Regardless of the cause, there are two solutions - reduce your memory usage or
increase your local memory space.  The latter is done by:

```pawn
    // Without YSI.
    #pragma dynamic 4096

    // With YSI.
    #define DYNAMIC_MEMORY 4096
```
)";
    case AMX_ERR_BOUNDS:       // index out of bounds
        throw PawnException(err)
            >> R"(
This is caused by attempting to access an array slot that doesn't exist, often
by not checking that a provided index is valid.  One very common cause is shown
below:

```pawn
    new gKills[MAX_PLAYERS];

    public OnPlayerDeath(playerid, killerid, reason)
    {
        ++gKills[killerid];
    }
```

Here the number of kills that the killer has is increased every time someone
dies.  However, when a player kills themselves, `killerid` is not valid, i.e.
it is `INVALID_PLAYER_ID`, because there is no killer.  If `MAX_PLAYERS` is
`100` and `INVALID_PLAYER_ID` is `65535` then this code attempts to increment
a value in slot `65535` of a `100` slot array.  This can't be done.

Another very common cause is an `OBOE` - an Off-By-One Error.  These can be
even more confusing, because they happen when you attempt to access slot `100`
in a `100` slot array (for example).  Surely that exists?  No, it doesn't.  If
an array has `2` slots, they are numbered `0`, and `1`.  `2` is the size of the
array, but not in the array.  Similarly with `[100]` the slots are numbered `0`
to `99`, `100` is not valid.  This code should use `<` in the loop condition,
not `<=`, so that the size of the array is excluded from the loop body:

```pawn
    new array[100];

    for (new i = 0; i <= sizeof (array); ++i)
    {
        array[i] = 10;
    }
```
)";
    case AMX_ERR_MEMACCESS:    // invalid memory access
        throw PawnException(err) >> "TODO: A good explanation of \"invalid memory access\".";
    case AMX_ERR_INVINSTR:     // invalid instruction
        throw PawnException(err) >> "TODO: A good explanation of \"invalid instruction\".";
    case AMX_ERR_STACKLOW:     // stack underflow
        throw PawnException(err) >> "TODO: A good explanation of \"stack underflow\".";
    case AMX_ERR_HEAPLOW:      // heap underflow
        throw PawnException(err) >> "TODO: A good explanation of \"heap underflow\".";
    case AMX_ERR_CALLBACK:     // no callback: or invalid callback
        throw PawnException(err) >> "TODO: A good explanation of \"no callback: or invalid callback\".";
    case AMX_ERR_NATIVE:       // native function failed
        throw PawnException(err) >> "TODO: A good explanation of \"native function failed\".";
    case AMX_ERR_DIVIDE:       // divide by zero
        throw PawnException(err)
            >> R"(
Something attempted to divide something else by `0`, which is mathematically
undefined:

```pawn
    main()
    {
        new
            a = 10,
            b; // Defaults to `0`.
        printf("%d", a / b); // `10 / 0` doesn't exist.
    }
```
)";
    case AMX_ERR_SLEEP:        // go into sleepmode - code can be restarted
        throw PawnException(err) >> "TODO: A good explanation of \"go into sleepmode - code can be restarted\".";
    case AMX_ERR_INVSTATE:     // invalid state for this access
        throw PawnException(err) >> "TODO: A good explanation of \"invalid state for this access\".";
    case AMX_ERR_MEMORY:
        throw PawnException(err) >> "TODO: A good explanation of \"AMX_ERR_MEMORY\".";
    case AMX_ERR_FORMAT:       // invalid file format
        throw PawnException(err) >> "TODO: A good explanation of \"invalid file format\".";
    case AMX_ERR_VERSION:      // file is for a newer version of the AMX
        throw PawnException(err) >> "The most common cause of this is compiling with `-O2`, which generates macro instructions we don't have.  Use `-O1`.";
    case AMX_ERR_NOTFOUND:     // function not found
        throw PawnException(err)
            >> R"(
One (or move) native function(s) was used in your script, but doesn't exist in the server.  See
above for the exact list of which.  This could be a typo in a native declaration:

```pawn
native Player_SetPosution(Player:p, Float:x, Float:y, Float:z);
```

It could be that the function hasn't been written yet, in which case contact the author or the
component you think it should be in (this is often obvious from the module name at the start of the
function name).  If you aren't sure, ask on forums or other chat systems.

Finally, it could be that the function has been removed.  This should be done with deprecation
first - check any warnings when compiling your code for messages like below and follow them:

```
warning 234: function is deprecated (symbol "AddPlayerClass") Use `Class_Add` instead.
```

Also check changelogs and component code - it could have been removed without warning, but this is bad.
)";

    case AMX_ERR_INDEX:        // invalid index parameter (bad entry point)
        throw PawnException(err)
            >> R"(
Your code is probably missing `main`.  Just add this:

```pawn
    main()
    {
    }
```
)";
    case AMX_ERR_DEBUG:        // debugger cannot run
        throw PawnException(err) >> "TODO: A good explanation of \"debugger cannot run\".";
    case AMX_ERR_INIT:         // AMX not initialized (or doubly initialized)
        throw PawnException(err) >> "TODO: A good explanation of \"AMX not initialized (or doubly initialized)\".";
    case AMX_ERR_USERDATA:     // unable to set user data field (table full)
        throw PawnException(err) >> "TODO: A good explanation of \"unable to set user data field (table full)\".";
    case AMX_ERR_INIT_JIT:     // cannot initialize the JIT
        throw PawnException(err) >> "TODO: A good explanation of \"cannot initialize the JIT\".";
    case AMX_ERR_PARAMS:       // parameter error
        throw PawnException(err) >> "TODO: A good explanation of \"parameter error\".";
    case AMX_ERR_DOMAIN:       // domain error: expression result does not fit in range
        throw PawnException(err) >> "TODO: A good explanation of \"domain error: expression result does not fit in range\".";
    case AMX_ERR_GENERAL:      // general error (unknown or unspecific error)
        throw PawnException(err) >> "TODO: A good explanation of \"eneral error (unknown or unspecific error)\".";
    }
}
*/

#if PAWN_CELL_SIZE == 16
#define CHARMASK (0xffffu << 8 * (2 - sizeof(char)))
#elif PAWN_CELL_SIZE == 32
#define CHARMASK (0xffffffffuL << 8 * (4 - sizeof(char)))
#elif PAWN_CELL_SIZE == 64
#define CHARMASK (0xffffffffffffffffuLL << 8 * (8 - sizeof(char)))
#else
#error Unsupported cell size
#endif

#define USENAMETABLE(hdr) \
    ((hdr)->defsize == sizeof(AMX_FUNCSTUBNT))
#define NUMENTRIES(hdr, field, nextfield) \
    (unsigned)(((hdr)->nextfield - (hdr)->field) / (hdr)->defsize)
#define GETENTRY(hdr, table, index) \
    (AMX_FUNCSTUB*)((unsigned char*)(hdr) + (unsigned)(hdr)->table + (unsigned)index * (hdr)->defsize)
#define GETENTRYNAME(hdr, entry)                                                             \
    (USENAMETABLE(hdr)                                                                       \
            ? (char*)((unsigned char*)(hdr) + (unsigned)((AMX_FUNCSTUBNT*)(entry))->nameofs) \
            : ((AMX_FUNCSTUB*)(entry))->name)

int AMXAPI amx_NumPublics(AMX* amx, int* number)
{
    AMX_HEADER* hdr = (AMX_HEADER*)amx->base;
    assert(hdr != NULL);
    assert(hdr->magic == AMX_MAGIC);
    assert(hdr->publics <= hdr->natives);
    *number = NUMENTRIES(hdr, publics, natives);
    return AMX_ERR_NONE;
}

int AMXAPI amx_GetPublic(AMX* amx, int index, char* funcname)
{
    AMX_HEADER* hdr;
    AMX_FUNCSTUB* func;

    hdr = (AMX_HEADER*)amx->base;
    assert(hdr != NULL);
    assert(hdr->magic == AMX_MAGIC);
    assert(hdr->publics <= hdr->natives);
    if (index >= (cell)NUMENTRIES(hdr, publics, natives))
        return AMX_ERR_INDEX;

    func = GETENTRY(hdr, publics, index);
    strcpy(funcname, GETENTRYNAME(hdr, func));
    return AMX_ERR_NONE;
}

__attribute__((noinline)) int amx_FindPublic_impl(AMX* amx, const char* name, int* index)
{
    // Attempt to find index in publics cache
    auto amxIter = cache.find(amx);
    const bool cacheExists = amxIter != cache.end();
    if (cacheExists) {
        const AMXCache& amxCache = *amxIter->second;
        if (amxCache.inited) {
            auto lookupIter = amxCache.publics.find(name);
            if (lookupIter != amxCache.publics.end()) {
                // https://github.com/IllidanS4/pawn-conventions/blob/master/guidelines.md#do-not-rely-on-consistency
                char pname[sNAMEMAX + 1];
                if (amx_GetPublic(amx, lookupIter->second, pname) == AMX_ERR_NONE && !strcmp(name, pname)) {
                    *index = lookupIter->second;
                    return AMX_ERR_NONE;
                }
            }
        }
    }

    // Cache miss; do the heavy search
    int first, last, mid, result;
    char pname[sNAMEMAX + 1];

    amx_NumPublics(amx, &last);
    last--; /* last valid index is 1 less than the number of functions */
    first = 0;
    /* binary search */
    while (first <= last) {
        mid = (first + last) / 2;
        amx_GetPublic(amx, mid, pname);
        result = strcmp(pname, name);
        if (result > 0) {
            last = mid - 1;
        } else if (result < 0) {
            first = mid + 1;
        } else {
            *index = mid;
            // Cache public index
            if (cacheExists) {
                AMXCache& amxCache = *amxIter->second;
                if (amxCache.inited) {
                    amxCache.publics[name] = mid;
                }
            }
            return AMX_ERR_NONE;
        } /* if */
    } /* while */
    /* not found, set to an invalid index, so amx_Exec() will fail */
    *index = INT_MAX;
    return AMX_ERR_NOTFOUND;
}

/// Pass-through to a noinline function to avoid adding complex instructions to the prologue that sampgdk can't handle
/// This should work in every case as both JMP and CALL are at least 5 bytes in size;
/// even in the minimal case it's guaranteed to contain a single JMP which is what sampgdk needs for a hook
int AMXAPI amx_FindPublic(AMX* amx, const char* name, int* index)
{
    return amx_FindPublic_impl(amx, name, index);
}

int AMXAPI amx_GetNativeByIndex(AMX const* amx, int index, AMX_NATIVE_INFO* ret)
{
    AMX_HEADER*
        hdr;
    int numnatives;
    AMX_FUNCSTUB*
        func;

    hdr = (AMX_HEADER*)amx->base;
    assert(hdr != NULL);
    assert(hdr->magic == AMX_MAGIC);
    assert(hdr->natives <= hdr->libraries);
    numnatives = NUMENTRIES(hdr, natives, libraries);

    if (index < numnatives) {
        func = (AMX_FUNCSTUB*)((unsigned char*)GETENTRY(hdr, natives, 0) + hdr->defsize * index);
        ret->func = (AMX_NATIVE)func->address;
        ret->name = GETENTRYNAME(hdr, func);
        return AMX_ERR_NONE;
    }

    return AMX_ERR_NOTFOUND;
}

int AMXAPI amx_MakeAddr(AMX* amx, cell* phys_addr, cell* amx_addr)
{
    AMX_HEADER* hdr;
    unsigned char* data;

    assert(amx != nullptr);
    hdr = reinterpret_cast<AMX_HEADER*>(amx->base);
    assert(hdr != nullptr);
    assert(hdr->magic == AMX_MAGIC);
    data = (amx->data != nullptr) ? amx->data : amx->base + static_cast<int>(hdr->dat);

    assert(amx_addr != nullptr);
    assert(phys_addr != nullptr);

    *amx_addr = static_cast<cell>(reinterpret_cast<unsigned char*>(phys_addr) - data);

    if ((*amx_addr >= amx->hea && *amx_addr < amx->stk) || *amx_addr < 0 || *amx_addr >= amx->stp) {
        *amx_addr = reinterpret_cast<cell>(nullptr);
        return AMX_ERR_MEMACCESS;
    } /* if */

    return AMX_ERR_NONE;
}

int AMXAPI amx_StrSize(const cell* cstr, int* length)
{
    /* Returns the number of cells this string requires, including NULL. */
    int len;
#if BYTE_ORDER == LITTLE_ENDIAN
    cell c;
#endif

    assert(length != nullptr);
    if (cstr == nullptr) {
        *length = 0;
        return AMX_ERR_PARAMS;
    } /* if */

    if (static_cast<ucell>(*cstr) > UNPACKEDMAX) {
        /* packed string */
        assert_static(sizeof(char) == 1);
        len = strlen(reinterpret_cast<char const*>(cstr)); /* find '\0' */
        //assert(check_endian());
#if BYTE_ORDER == LITTLE_ENDIAN
        /* on Little Endian machines, toggle the last bytes */
        c = cstr[len / sizeof(cell)]; /* get last cell */
        len = len - len % sizeof(cell); /* len = multiple of "cell" bytes */
        while ((c & CHARMASK) != 0) {
            len++;
            c <<= 8 * sizeof(char);
        } /* if */
#endif
        *length = CEILDIV(len + 1, PAWN_CELL_SIZE / 8);
    } else {
        for (len = 0; cstr[len] != 0; len++)
            /* nothing */;
        *length = len + 1;
    } /* if */
    return AMX_ERR_NONE;
}
