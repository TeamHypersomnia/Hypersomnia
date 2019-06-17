#include "augs/log_path_getters.h"
#include "augs/string/typesafe_sprintf.h"
#include "augs/window_framework/print_x_devices.h"
#include "augs/filesystem/file.h"

void print_classes(std::string& total_info, Display *display, XIAnyClassInfo **classes, int num_classes) {
	auto pf = [&total_info](auto&&... args){
		total_info += typesafe_sprintf(std::forward<decltype(args)>(args)...);
	};

	int i, j;

	for (i = 0; i < num_classes; i++)
	{
		pf("\t\tClass originated from: %x\n", classes[i]->sourceid);
		switch(classes[i]->type)
		{
			case XIButtonClass:
			{
				XIButtonClassInfo *b = (XIButtonClassInfo*)classes[i];
				pf("\t\tButtons supported: %x\n", b->num_buttons);
				pf("\t\tButton labels:");
				for (j = 0; j < b->num_buttons; j++)
				pf(" '%x'", (b->labels[j]) ?  XGetAtomName(display, b->labels[j]) : "None");
				pf("\n");
				pf("\t\tButton state:");
				for (j = 0; j < b->state.mask_len * 8; j++)
				if (XIMaskIsSet(b->state.mask, j))
				pf(" %x", j);
				pf("\n");

			}
			break;
			case XIKeyClass:
			{
				XIKeyClassInfo *k = (XIKeyClassInfo*)classes[i];
				pf("\t\tKeycodes supported: %x\n", k->num_keycodes);
			}
			break;
			case XIValuatorClass:
			{
				XIValuatorClassInfo *v = (XIValuatorClassInfo*)classes[i];
				pf("\t\tDetail for Valuator %x:\n", v->number);
				pf("\t\t  Label: '%x'\n", (v->label) ?  XGetAtomName(display, v->label) : "None");
				pf("\t\t  Range: %x - %x\n", v->min, v->max);
				pf("\t\t  Resolution: %x units/m\n", v->resolution);
				pf("\t\t  Mode: %x\n", v->mode == XIModeAbsolute ? "absolute" :
				"relative");
				if (v->mode == XIModeAbsolute)
				pf("\t\t  Current value: %x\n", v->value);
			}
			break;
		}
	}
}

void print_input_devices(Display* const display) {
	std::string total_info = "Device info start.\n";

	int ndevices;

	if (const auto info = XIQueryDevice(display, XIAllDevices, &ndevices)) {
		const char *type = "";

		auto pf = [&total_info](auto&&... args){
			total_info += typesafe_sprintf(std::forward<decltype(args)>(args)...);
		};

		for(int i = 0; i < ndevices; i++) {
			auto dev = &info[i];

			pf("'%x' (%x)\n", dev->name, dev->deviceid);
			switch(dev->use) {
				case XIMasterPointer: type = "master pointer"; break;
				case XIMasterKeyboard: type = "master keyboard"; break;
				case XISlavePointer: type = "slave pointer"; break;
				case XISlaveKeyboard: type = "slave keyboard"; break;
				case XIFloatingSlave: type = "floating slave"; break;
			}

			pf(" - is a %x\n", type);
			pf(" - current pairing/attachment: %x\n", dev->attachment);

			if (!dev->enabled) {
				pf(" - this device is disabled!\n");
			}

			print_classes(total_info, display, dev->classes, dev->num_classes);
		}

		XIFreeDeviceInfo(info);
	}
	else {
		total_info += "Couldn't query input device info.\n";
	}

	total_info += "\nDevice info end.";

	augs::save_as_text(get_path_in_log_files("input_devices.txt"), total_info);
}
