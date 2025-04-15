#include "engineclient.h"
#include "..\..\utils\hook\hooks.h"
CEconItemAttributeDefinition* CEconItemSchema::get_attribute_definition_by_name(const char* name) {
	return hooks::get_attribute_definition_by_name(&name);
}

