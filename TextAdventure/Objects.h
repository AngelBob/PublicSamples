#pragma once

#include "json.hpp"  // for the json handling
using json = nlohmann::json;

#include "ResourceLoader.hpp"

// Parser traits
#include "Parser.h"
using ParserT = typename Parser::SimpleParser;
using ParserStatusT = typename Parser::ParserStatus;
using ParserStateT = typename Parser::ParseState;

#include "InGameObject.h"
#include "Game.h"

