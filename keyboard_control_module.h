#ifndef KEYBOARD_CONTROL_MODULE_H
#define	KEYBOARD_CONTROL_MODULE_H

struct AxisKey {
	regval axis_index;
	regval pressed_value;
	regval unpressed_value;
};

class KeyboardControlModule : public ControlModule {
	AxisData *robot_axis;
	int COUNT_AXIS;
	
	bool is_error_init;
	
	std::map<std::string, regval> axis_names;
	std::map<WORD, AxisKey*> axis_keys;
	std::map<regval, AxisData*> axis;

	public:
		KeyboardControlModule();
		virtual int init();
		virtual AxisData* getAxis(int *count_axis);
		virtual void execute(sendAxisState_t sendAxisState);
		virtual void final() {};
		virtual void destroy();
		virtual ~KeyboardControlModule() {}
};

#endif	/* KEYBOARD_CONTROL_MODULE_H */