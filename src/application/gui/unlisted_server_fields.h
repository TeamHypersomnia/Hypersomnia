#pragma once
#include <array>
#include <optional>
#include <string>
#include <utility>
#include "augs/graphics/rgba.h"
#include "application/setups/client/connect_target.h"

/*
	An unlisted server's address, key and name as typed into Connect (its Unlisted server tab) or Add an unlisted server,
	and the rules both take them from the clipboard by:
	- auto-paste: "<address>,0x<key>[,<name>]" (see connect_target::find_server_identifiers) in the clipboard when the
	  window opens or gets the focus back - a clipboard taken once, so what the player changed since stays;
	- a paste (Ctrl+V) taken whole before a field takes it, and the field left: the identifiers into any field fill them
	  all; a key into the key's field; an address into the address's, then on to the key's while it has no good key.
	  Anything else is pasted as usual.
*/

struct unlisted_server_fields {
	std::string address;
	std::string key;
	std::string name;

	/*
		Set by the window as it draws them: which field had the keyboard last frame.
		Read by the window: the key's field should get the keyboard next.
	*/

	bool address_active = false;
	bool key_active = false;
	bool focus_key_next = false;

	std::string last_clipboard;
	std::optional<std::array<std::string, 3>> autopasted;

	static std::optional<connect_target::server_identifiers> find(const std::string& text);

	/*
		Whatever field was being edited takes the new values at once.
	*/

	bool take(const std::string& text);
	bool autopaste(const char* clipboard);

	/*
		Only while the window drawing the fields has the focus. True when the identifiers were taken whole.
	*/

	bool take_paste();

	std::optional<augs::ed25519_public_key_type> typed_key() const;
	std::string typed_address() const;
	bool ready() const;
	bool is_autopasted() const;

	/*
		One line under the fields - always there, so the layout never jumps: what is wrong, else that they came from
		the clipboard (until one is changed), else OK once they are right, else how to fill them at once.
		Without hints (editing a favorite) only what is wrong.
	*/

	void status_line(
		const std::optional<std::pair<std::string, rgba>>& problem,
		bool hints,
		const char* hint
	) const;
};
