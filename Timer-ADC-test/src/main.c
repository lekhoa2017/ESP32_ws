#include "mgos.h"
#include "mgos_mqtt.h"
#include "mgos_adc.h"
#include "esp32/esp32_adc.h"

// Variable declaration
int count1 = 0;
int count2 = 0;
int count3 = 0;
int rawReading1 = 0; 
float maxValue1 = -10; // initialise max min value of a 10 ms window
float minValue1 = 10000;
float filterData1 = 0;   // filter data at t
float filterData1_1 = 0; // at t-1
float filterData1_2 = 0; // at t-2
float minDataOut1 =20; // min data in 100ms 
float averageData1 = 0; // average current in 60s
double Gain = 1.2;     // Adjusment gain
float sumData1 = 0;   // Sum of current in 60s


static void led_timer_cb(void *arg) {
  //bool val = mgos_gpio_toggle(2);
  int rawReading1 = mgos_adc_read(36);
  //LOG(LL_INFO, ("Analog data %d", data));
  if (maxValue1 < rawReading1) { maxValue1 = rawReading1;}
  if (minValue1 > rawReading1) { minValue1 = rawReading1;}
  count1++;
  
  if (count1>=10)  // 10ms window
  {
    maxValue1 = maxValue1*3.3/4095 -2.32;  // convert to voltage and offset 2.32V
    minValue1 = minValue1*3.3/4095 -2.32;
    if ((maxValue1 - minValue1) <0.05) {maxValue1= 0;} //Filter raw noise
    filterData1  = maxValue1 * 5 * Gain;  // Convert to Amp
    filterData1 = 0.6*filterData1 + 0.3*filterData1_1 + 0.1*filterData1_2; // Low pass filter
    filterData1_2 = filterData1_1;
    filterData1_1 = filterData1;
    if (filterData1< minDataOut1) {minDataOut1 = filterData1;}
    if (minDataOut1<0) {minDataOut1 = 0;}
    count2++;
    // Reset value for new 10ms window
    maxValue1 = -10;
    minValue1 = 10000;
    count1 = 0;
  }

  if (count2>=10)  //100ms
  {
    sumData1 = sumData1 + minDataOut1;
    LOG(LL_INFO, ("Instant current %fA , time stick %.2lf", minDataOut1,mgos_uptime()));
    count3++;
    minDataOut1 = 20;
    count2 = 0;

  }

  if (count3>=600) // every 60s
  {
    averageData1 = sumData1/count3;
    LOG(LL_INFO, ("Average current %fA , time stick %.2lf", averageData1, mgos_uptime()));

    sumData1 = 0;
    count3 = 0;
  }
 
  (void) arg;
}

static void net_cb(int ev, void *evd, void *arg) {
  switch (ev) {
    case MGOS_NET_EV_DISCONNECTED:
      LOG(LL_INFO, ("%s", "Net disconnected"));
      break;
    case MGOS_NET_EV_CONNECTING:
      LOG(LL_INFO, ("%s", "Net connecting..."));
      break;
    case MGOS_NET_EV_CONNECTED:
      LOG(LL_INFO, ("%s", "Net connected"));
      break;
    case MGOS_NET_EV_IP_ACQUIRED:
      LOG(LL_INFO, ("%s", "Net got IP address"));
      break;
  }

  (void) evd;
  (void) arg;
}



static void button_cb(int pin, void *arg) {
  char topic[100], message[100];
  struct json_out out = JSON_OUT_BUF(message, sizeof(message));
  snprintf(topic, sizeof(topic), "/devices/%s/events",
           mgos_sys_config_get_device_id());
  json_printf(&out, "{total_ram: %lu, free_ram: %lu}",
              (unsigned long) mgos_get_heap_size(),
              (unsigned long) mgos_get_free_heap_size());
  bool res = mgos_mqtt_pub(topic, message, strlen(message), 1, false);
  LOG(LL_INFO, ("Pin: %d, published: %s", pin, res ? "yes" : "no"));
  (void) arg;
}

enum mgos_app_init_result mgos_app_init(void) {
  mgos_gpio_set_mode(2, MGOS_GPIO_MODE_OUTPUT);
  bool pin_is_enabled = mgos_adc_enable(36);
 
  mgos_set_timer(1, MGOS_TIMER_REPEAT, led_timer_cb, NULL);

  /* Publish to MQTT on button press */
  mgos_gpio_set_button_handler(mgos_sys_config_get_pins_button(),
                               MGOS_GPIO_PULL_UP, MGOS_GPIO_INT_EDGE_NEG, 20,
                               button_cb, NULL);
                               

  /* Network connectivity events */
  mgos_event_add_group_handler(MGOS_EVENT_GRP_NET, net_cb, NULL);
  return MGOS_APP_INIT_SUCCESS;
}
