#include "definition_interface.h"

definition_interface::definition_interface(full_entity_definition& definition) : def(&definition) {

}

definition_interface::definition_interface(entity_id _id) : id(_id), def(nullptr) {

}
