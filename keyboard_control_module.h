#ifndef KEYBOARD_CONTROL_MODULE_H
#define	KEYBOARD_CONTROL_MODULE_H

struct AxisKey {
	system_value axis_index;
	variable_value pressed_value;
	variable_value unpressed_value;
};

class KeyboardControlModule : public ControlModule {
	AxisData **robot_axis;
	unsigned int COUNT_AXIS;
	colorPrintf_t *colorPrintf;
	
	bool is_error_init;
	
	std::map<std::string, system_value> axis_names;
	std::map<WORD, AxisKey*> axis_keys;
	std::map<system_value, AxisData*> axis;

	public:
		KeyboardControlModule();
		const char *getUID();
		void prepare(colorPrintf_t *colorPrintf_p, colorPrintfVA_t *colorPrintfVA_p);
		int init();
		AxisData** getAxis(unsigned int *count_axis);
		void execute(sendAxisState_t sendAxisState);
		void final() {};
		void destroy();
		~KeyboardControlModule() {}
};

#endif	/* KEYBOARD_CONTROL_MODULE_H */