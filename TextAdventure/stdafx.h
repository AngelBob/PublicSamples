#pragma once

#include <array>
#include <cassert>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include "windows.h"
#include "resource.h" // for resource definition

#include "json.hpp"
using json = nlohmann::json;