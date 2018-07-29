#pragma once
#include "game/messages/will_soon_be_deleted.h"
#include "game/messages/queue_deletion.h"

/* Helper functions */

void make_deletion_queue(const const_entity_handle, deletion_queue& q);
void make_deletion_queue(const destruction_queue&, deletion_queue&, const cosmos& cosm);

deletion_queue make_deletion_queue(const const_entity_handle);
deletion_queue make_deletion_queue(const destruction_queue&, const cosmos& cosm);

void reverse_perform_deletions(const deletion_queue&, cosmos& cosm);
void delete_entity_with_children(const entity_handle);
