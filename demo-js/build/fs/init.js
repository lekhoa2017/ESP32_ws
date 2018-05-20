load('api_config.js');
load('api_events.js');
load('api_gpio.js');
load('api_mqtt.js');
load('api_net.js');
load('api_sys.js');
load('api_timer.js');
load('api_adc.js');

let led = 2;
let raw1 = 0;
let raw2 = 0;
let count1 =0;
let count2 =0;
let count3=0;
let max1=-10;
let min1=10000;
let result1=0;
let result1_1=0;
let result1_2=0;
let minOut1=20;
let average1=0;
let Gain=1.2;
let sum1=0;

let button = Cfg.get('pins.button');
let topic = '/devices/' + Cfg.get('device.id') + '/events';
//let topic ='projects/helibot-cloud/topics/iot1';
ADC.enable(36);

print('LED GPIO:', led, 'button GPIO:', button);

let getInfo = function() {
  return JSON.stringify({
    total_ram: Sys.total_ram(),
    free_ram: Sys.free_ram(),
    sensor1: average1
  });
};


// Blink built-in LED every second
GPIO.set_mode(led, GPIO.MODE_OUTPUT);

Timer.set(10 /* 1 msec */, Timer.REPEAT, function() {
  
  // Read sensor data everything 1ms
  //raw1 = ADC.read(36);
  count1++;
  if (count1>100)
  {
    print('raw', raw1);
    count1=0;
  }
  //print('raw', raw1);
/*
  if (max1<raw1) {max1=raw1;}
  if (min1>raw1) {min1=raw1;}
  
  count1++;
  if (count1>=10) // every 10ms
  {
    max1=(max1*3.3)/4095-2.32;
    min1=(min1*3.3)/4095-2.32;
    if((max1-min1)<0.05){max1=0;}
    result1=max1*5*Gain;
    //filter1
    result1=0.6*result1+0.3*result1_1+0.1*result1_2;
    result1_2=result1_1;
    result1_1=result1;
    if (result1<minOut1) {minOut1=result1;}
    if (minOut1<0) {minOut1=0;}
    count2++;
    
    
    max1=-10;
    min1=10000;
    count1=0;
  }
  
  if (count2>=10) //every 100ms
  {
    //print('Sensor1', minOut1); // display sensor reading every 100ms
    sum1=sum1+minOut1;
    count3++;
    print('count3 ', count3);
    minOut1=20;
    count2=0;
  }
  
  if (count3>=60) // every 60s
  {
    average1=sum1/count3;
    //print(value ? 'Tick' : 'Tock', 'uptime:', Sys.uptime(), getInfo());
    let message = getInfo();
    let ok = MQTT.pub(topic, message, 1);
    print('Published:', ok, topic, '->', message);
    sum1=0;
    count3=0;
  }*/
}, null);




// Publish to MQTT topic on a button press. Button is wired to GPIO pin 0
GPIO.set_button_handler(button, GPIO.PULL_UP, GPIO.INT_EDGE_NEG, 20, function() {
  //let message = getInfo();
  //let ok = MQTT.pub(topic, message, 1);
  //print('Published:', ok, topic, '->', message);
  let value = GPIO.toggle(led);
}, null);




// Monitor network connectivity.
Event.addGroupHandler(Net.EVENT_GRP, function(ev, evdata, arg) {
  let evs = '???';
  if (ev === Net.STATUS_DISCONNECTED) {
    evs = 'DISCONNECTED';
  } else if (ev === Net.STATUS_CONNECTING) {
    evs = 'CONNECTING';
  } else if (ev === Net.STATUS_CONNECTED) {
    evs = 'CONNECTED';
  } else if (ev === Net.STATUS_GOT_IP) {
    evs = 'GOT_IP';
  }
  print('== Net event:', ev, evs);
}, null);

