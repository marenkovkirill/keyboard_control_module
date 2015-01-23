#ifndef KEYBOARD_CONTROL_MODULE_H
#define	KEYBOARD_CONTROL_MODULE_H

class KeyboardControlModule : public ControlModule {
	AxisData *robot_axis;
	bool is_error_init;

	public:
		KeyboardControlModule();
		virtual int init();
		virtual AxisData* getAxis(int *count_functions);
		virtual void execute(sendAxisState_t sendAxisState);
		virtual void final() {};
		virtual void destroy();
		virtual ~KeyboardControlModule() {}
};

struct AxisKey {
	regval axis_index;
	regval pressed_value;
	regval unpressed_value;
};

extern "C" {
	__declspec(dllexport) ControlModule* getControlModuleObject();
}

#endif	/* KEYBOARD_CONTROL_MODULE_H */