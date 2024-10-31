#define CATCH_CONFIG_MAIN

#include <libtcod-fov/fov.h>
#include <libtcod-fov/logging.h>

#include <catch2/catch_all.hpp>
#include <clocale>
#include <cstddef>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <utility>

/// Captures libtcod log output on tests.
struct HandleLogging : Catch::EventListenerBase {
  using EventListenerBase::EventListenerBase;  // inherit constructor
  /// Register logger before each test.
  void testCaseStarting(Catch::TestCaseInfo const&) override {
    TCODFOV_set_log_level(TCODFOV_log_DEBUG);
    TCODFOV_set_log_callback(&catch_log, nullptr);
  }
  static void catch_log(const TCODFOV_LogMessage* message, void*) {
    INFO("libtcod:" << message->source << ":" << message->lineno << ":" << message->level << ":" << message->message);
  }
};
CATCH_REGISTER_LISTENER(HandleLogging)

std::ostream& operator<<(std::ostream& out, const std::array<ptrdiff_t, 2>& data) {
  return out << '{' << data.at(0) << ',' << ' ' << data.at(1) << '}';
}
