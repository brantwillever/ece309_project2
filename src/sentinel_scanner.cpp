#include "core/sentinel_scanner.h"

// TODO: implement the stream scanner in the next phase.
SentinelScanner::SentinelScanner(std::string sentinel) {
    sentinel_value = sentinel;
    pending_text.clear();
}

SentinelScanner::Out SentinelScanner::feed(std::string_view chunk) {
    Out result;
    result.safe_text = std::string(chunk);
    result.sentinel_found = false;
    (void)chunk;
    return result;
}

SentinelScanner::Out SentinelScanner::flush() {
    Out result;
    result.safe_text = "";
    result.sentinel_found = false;
    return result;
}
