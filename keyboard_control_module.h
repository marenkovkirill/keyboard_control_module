#ifndef KEYBOARD_CONTROL_MODULE_H
#define	KEYBOARD_CONTROL_MODULE_H

struct AxisKey {
	regval axis_index;
	regval pressed_value;
	regval unpressed_value;
};

class KeyboardControlModule : public ControlModule {
	AxisData **robot_axis;
	int COUNT_AXIS;
	
	bool is_error_init;
	
	std::map<std::string, regval> axis_names;
	std::map<WORD, AxisKey*> axis_keys;
	std::map<regval, AxisData*> axis;

	public:
		KeyboardControlModule();
		const char *getUID();
		int init();
		AxisData** getAxis(int *count_axis);
		void execute(sendAxisState_t sendAxisState);
		void final() {};
		void destroy();
		~KeyboardControlModule() {}
};

#endif	/* KEYBOARD_CONTROL_MODULE_H */