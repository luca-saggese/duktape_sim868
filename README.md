# duktape_sim868
Javascript (Ducktape) implementation for sim868

you can call all eat function directly from the serial line

You will need the EAT for sim868 and compile from there.

it will autorun the index.js file in the SD.

simple index.js:
```javascript
function event_callback(event){
	eat_trace(JSON.stringify(event));
	if(event.event==1 && event.timer_id==1){
		eat_trace(eat_gps_nmea_info_output(0));
		eat_timer_start(1,2000);
	}
}

eat_timer_start(1,2000);
```
