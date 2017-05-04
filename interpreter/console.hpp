#pragma once

#include "deviser.hpp"

std::shared_ptr<module> make_console_module(shared_ptr<lexicalscope> top_level_scope);
