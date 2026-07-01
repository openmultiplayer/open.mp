
add_subdirectory(Actors)
add_subdirectory(CAPI)
add_subdirectory(Checkpoints)
add_subdirectory(Classes)
add_subdirectory(Console)
add_subdirectory(Dialogs)

# Fixes
if(BUILD_FIXES_COMPONENT)
	add_subdirectory(Fixes)
endif()

add_subdirectory(GangZones)
add_subdirectory(Menus)
add_subdirectory(Objects)
add_subdirectory(Pickups)
add_subdirectory(Recordings)
add_subdirectory(TextDraws)
add_subdirectory(TextLabels)
add_subdirectory(Timers)
add_subdirectory(Variables)
add_subdirectory(Vehicles)
add_subdirectory(NPCs)

# Pawn
if(BUILD_PAWN_COMPONENT)
	add_subdirectory(Pawn)
endif()

# SQLite
if(BUILD_SQLITE_COMPONENT)
	add_subdirectory(Databases)
endif()

if(BUILD_UNICODE_COMPONENT)
	add_subdirectory(Unicode)
endif()

# Legacy
if(BUILD_LEGACY_COMPONENTS)
add_subdirectory(LegacyNetwork)
add_subdirectory(LegacyConfig)
endif()

# Test
if(BUILD_TEST_COMPONENTS)
	add_subdirectory(DatabasesTest)
	add_subdirectory(TestComponent)
endif()

add_subdirectory(CustomModels)
