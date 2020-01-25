uint16_t gap_init_service(char *, uint16_t psm, uint8_t);
bool gap_init_hardware();
static void gap_event_handle(uint16_t, uint16_t);

static tL2CAP_ERTM_INFO ertm_info = {0};
static tL2CAP_CFG_INFO cfg_info = {0};

extern uint8_t gamepad_btaddr[BD_ADDR_LEN];
