
function bear_notify_cb(state, ip){
  eat_trace('bear_notify_cb state:' + state + ' ip:'+ ip);
}

function soc_notify_cb(s, event, result, ack_size){
  eat_trace('soc_notify_cb event:' + event + ' result:'+ result);
}

function modem_callback(buffer){
	eat_trace(JSON.stringify(buffer));
}

function event_callback(event){
	eat_trace(JSON.stringify(event));
	if(event.event==1 && event.timer_id==1){
		eat_trace(eat_gps_nmea_info_output(0));
		eat_timer_start(1,2000);
	}
}

function eat_sms_new_message_cb(id){
	eat_trace('new sms arrived ' + id);
}

function eat_sms_read_cb(msg){
	eat_trace('message read ' + JSON.stringify(msg));
}

function test_acc(){
	eat_trace(eat_mma_init());
	eat_trace(eat_mma_config(2,0));
	eat_trace(JSON.stringify(eat_mma_read(2)));
}

//eat_timer_start(1,2000);
function init(){
	//eat_trace('open bearer ret: ' + eat_gprs_bearer_open('TM', '', ''));
	//eat_gprs_bearer_hold();
	eat_ftp_init('TM','','');
	eat_ftp_get("ftp.thingsmind.com","21", "lvx", "existence", "app.bin","",1);
	eat_set_sms_sc('+447797704000')
	eat_trace(eat_send_text_sms("+393475541920","test"))
	eat_modem_write("AT+CNMI=2,2,0,0,0\r") //abilita gli sms in ingresso

}

const SOC_WOULDBLOCK        = -2;
const SOC_READ    = 0x01;  /* Notify for read */
const SOC_WRITE   = 0x02;  /* Notify for write */
const SOC_ACCEPT  = 0x04;  /* Notify for accept */
const SOC_CONNECT = 0x08;  /* Notify for connect */
const SOC_CLOSE   = 0x10;   /* Notify for close */
const SOC_ACKED   = 0x20;  /* Notify for acked */
const SOC_OOBINLINE     = 0x01 << 0;  /* not support yet */
const SOC_LINGER        = 0x01 << 1;  /* linger on close */
const SOC_NBIO          = 0x01 << 2;  /* Nonblocking */
const SOC_ASYNC         = 0x01 << 3;  /* Asynchronous notification */   
 
const SOC_NODELAY       = 0x01 << 4;  /* disable Nagle algorithm or not */
const SOC_KEEPALIVE     = 0x01 << 5;  /* enable/disable the keepalive */
const SOC_RCVBUF        = 0x01 << 6;  /* set the socket receive buffer size */
const SOC_SENDBUF       = 0x01 << 7;  /* set the socket send buffer size */

const SOC_NREAD         = 0x01 << 8;  /* no. of bytes for read, only for soc_getsockopt */
const SOC_PKT_SIZE      = 0x01 << 9; /* datagram max packet size */
const SOC_SILENT_LISTEN = 0x01 << 10; /* SOC_SOCK_SMS property */
const SOC_QOS           = 0x01 << 11; /* set the socket qos */

const SOC_TCP_MAXSEG    = 0x01 << 12; /* set the max segmemnt size */
const SOC_IP_TTL        = 0x01 << 13; /* set the IP TTL value */
const SOC_LISTEN_BEARER = 0x01 << 14; /* enable listen bearer */
const SOC_UDP_ANY_FPORT = 0x01 << 15; /* enable UDP any foreign port */

const SOC_WIFI_NOWAKEUP = 0x01 << 16; /* send packet in power saving mode */
const SOC_UDP_NEED_ICMP = 0x01 << 17; /* deliver NOTIFY(close) for ICMP error */
const SOC_IP_HDRINCL    = 0x01 << 18;  /* IP header included for raw sockets */
const SOC_IPSEC_POLICY      = 0x01 << 19; /* ip security policy */
const SOC_TCP_ACKED_DATA  = 0x01 << 20;  /* TCPIP acked data */
const SOC_TCP_DELAYED_ACK = 0x01 << 21; /* TCP delayed ack */
const SOC_TCP_SACK        = 0x01 << 22; /* TCP selective ack */
const SOC_TCP_TIME_STAMP  = 0x01 << 23;  /* TCP time stamp */
const SOC_TCP_ACK_MSEG  = 0x01 << 24;  /* TCP ACK multiple segment */

const SOC_SUCCESS           = 0;
const TRUE =1;

function connectSocket(){
	//eat_soc_notify_register(soc_notify_cb);
	var socket_id = eat_soc_create(0,0);
	if(socket_id < 0)
		eat_trace("eat_soc_create return error " + socket_id);
	val = (SOC_READ | SOC_WRITE | SOC_CLOSE | SOC_CONNECT|SOC_ACCEPT);
	ret = eat_soc_setsockopt(socket_id,SOC_ASYNC,val);
	if (ret != SOC_SUCCESS)
		eat_trace("eat_soc_setsockopt 1 return error :" + ret);

	val = TRUE;
	ret = eat_soc_setsockopt(socket_id, SOC_NBIO, val);
	if (ret != SOC_SUCCESS)
		eat_trace("eat_soc_setsockopt 2 return error :" + ret);

	val = TRUE;
	ret = eat_soc_setsockopt(socket_id, SOC_NODELAY, val);
	if (ret != SOC_SUCCESS)
		eat_trace("eat_soc_setsockopt 3 return error :" + ret);

	ret = eat_soc_getsockopt(socket_id, SOC_NODELAY, val);
	if (ret != SOC_SUCCESS)
		eat_trace("eat_soc_getsockopt  return error :" + ret);
	else 
		eat_trace("eat_soc_getsockopt return %d" + val);

	ret = eat_soc_connect(socket_id,'116,247,119,165:5107'); 
	if(ret >= 0){
		eat_trace("NEW Connection ID is :" + ret);
	}
	else if (ret == SOC_WOULDBLOCK) {
		eat_trace("Connection is in progressing");
	}
	else {
		eat_trace("Connect return error:" + ret);
	}
}