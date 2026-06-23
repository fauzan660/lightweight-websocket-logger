#pragma once
#include "nlohmann/json.hpp"
#include "ws.h"
using json = nlohmann::json;

void handle_action(Action action, json json_request);
