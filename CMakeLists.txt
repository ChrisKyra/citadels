cmake_minimum_required(VERSION 3.20)
project(citadels LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Debug for single-config generators (Make, Ninja)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Shared warning flags
add_library(project_warnings INTERFACE)
if(MSVC)
  # /W4 is the Visual Studio counterpart of -Wall -Wextra; /utf-8 reads source files and string
  # literals as UTF-8 (the GUI protocol carries UTF-8 text).
  target_compile_options(project_warnings INTERFACE /W4 /utf-8)
else()
  target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

# Sanitizers (Debug builds, GCC/Clang only)
set(SANITIZER "address" CACHE STRING "Sanitizer to use: address, thread, none")
set_property(CACHE SANITIZER PROPERTY STRINGS address thread none)

add_library(project_sanitizers INTERFACE)
unset(_san_flags)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  if(SANITIZER STREQUAL "address")
    set(_san_flags -fsanitize=address,undefined)
  elseif(SANITIZER STREQUAL "thread")
    set(_san_flags -fsanitize=thread)
  elseif(NOT SANITIZER STREQUAL "none")
    message(FATAL_ERROR "Unknown SANITIZER '${SANITIZER}' (use address, thread, or none)")
  endif()

  if(_san_flags)
    target_compile_options(project_sanitizers INTERFACE
      $<$<CONFIG:Debug>:${_san_flags}>
      $<$<CONFIG:Debug>:-fno-omit-frame-pointer>)
    target_link_options(project_sanitizers INTERFACE
      $<$<CONFIG:Debug>:${_san_flags}>)
  endif()
endif()

# 1. Library: all game logic
# Every .cpp file with game logic goes into this list.
# (card.hpp and color.hpp are header-only, so only hand.cpp is compiled.)
add_library(citadels_core
  src/cpp/hand.cpp
  src/cpp/city.cpp
  src/cpp/player.cpp
  src/cpp/random.cpp
  src/cpp/character_id.cpp
  src/cpp/card_loader.cpp
  src/cpp/characters.cpp
  src/cpp/character_factory.cpp
  src/cpp/character_deck.cpp
  src/cpp/scripted_ui.cpp
  src/cpp/console_ui.cpp
  src/cpp/protocol_ui.cpp
  src/cpp/game_state.cpp
  src/cpp/turn.cpp
  src/cpp/character_draft.cpp
  src/cpp/round.cpp
  src/cpp/score_calculator.cpp
  src/cpp/game.cpp
  src/cpp/district_ability.cpp
  src/cpp/ability_factory.cpp
  )
target_include_directories(citadels_core PUBLIC src/hpp)
target_link_libraries(citadels_core PRIVATE project_warnings project_sanitizers)

# 2. Game executable: only main.cpp
add_executable(citadels src/cpp/main.cpp)
target_link_libraries(citadels PRIVATE citadels_core project_warnings project_sanitizers)

# Without a tests/ folder (the published game repository) we are done here: no Catch2 download.
if(NOT IS_DIRECTORY "${CMAKE_SOURCE_DIR}/tests")
  return()
endif()

# 3. Tests (Catch2 v3)
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.1
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

enable_testing()
add_executable(tests
  tests/test_color.cpp
  tests/test_card.cpp
  tests/test_card_loader.cpp
  tests/test_hand.cpp
  tests/test_city.cpp
  tests/test_character_id.cpp
  tests/test_character_powers.cpp
  tests/test_player.cpp
  tests/test_round_powers.cpp
  tests/test_random.cpp
  tests/test_deck.cpp
  tests/test_character.cpp
  tests/test_character_factory.cpp
  tests/test_character_deck.cpp
  tests/test_district_deck.cpp
  tests/test_user_interface.cpp
  tests/test_scripted_ui.cpp
  tests/test_console_ui.cpp
  tests/test_protocol_ui.cpp
  tests/test_game_state.cpp
  tests/test_turn_context.cpp
  tests/test_character_hooks.cpp
  tests/test_turn.cpp
  tests/test_round_state.cpp 
  tests/test_character_draft.cpp
  tests/test_round.cpp
  tests/test_score_calculator.cpp
  tests/test_game.cpp
  )

target_link_libraries(tests PRIVATE
  citadels_core project_warnings project_sanitizers Catch2::Catch2WithMain)
# The loader tests read data/districts.csv; this tells them where the data folder is.
target_compile_definitions(tests PRIVATE CITADELS_DATA_DIR="${CMAKE_SOURCE_DIR}/data")

catch_discover_tests(tests)