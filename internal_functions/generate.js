/*
duk_ret_t my_eat_gps_nmea_info_output(duk_context *ctx) {
    int id = (EatTimer_enum)duk_get_int(ctx, 0);
    eat_gps_nmea_info_output(id, gps_info_buf,NMEA_BUFF_SIZE);
    duk_push_string(ctx, gps_info_buf);
    return 1;  
  }

duk_push_c_function(ctx, my_eat_gps_power_req, 1);
  duk_put_global_string(ctx, "eat_gps_power_req");


unsigned short(*const  eat_uart_write )(EatUart_enum uart, const unsigned char *data, unsigned short len)

 */
const fs=require('fs');
let func_list=[];

const ws=fs.createWriteStream('./internal_functions/internal.c');
//ws.write('static register_bindings(duk_context *ctx){');

fs.readFile('./internal_functions/list.txt', (err, data) => {
  if (err) throw err;
  let funcs = data.toString().split("\n");

  funcs.forEach(f=>{
    const parts = /(.*?)\s+(.*?)\s*\((.*?)\)/.exec(f);
    //const parts = /(.*?)\((.*?)[\s\t]+(.*?)\s*\)\((.*?)\)/.exec(f);
    const ret = parts[1];
    const name = parts[2];
    const params = parts[3].split(',');
    ws.write('duk_ret_t _'+ name +'(duk_context *ctx) {\n');
    let cnt=0;
    params.forEach(p=>{
      if(p!='' && p!='void'){
        const a=/(.*)\s(.*)/.exec(p.trim());
        ws.write('\t' + a[1] + ' ' + a[2] + ' = (' + a[1] +') duk_get_int(ctx, '+ cnt++ +');\n');
      }
    });
    func_list.push([name,cnt])
    ws.write('\t' + ret +' ret = ' + name +'(');
    params.forEach(p=>{
      if(p!='' && p!='void'){
        const a=/(.*)\s(.*)/.exec(p.trim());
        ws.write(a[2]);
        if(cnt-->1) ws.write(', ');
      }
    });
    ws.write(');\n\tduk_push_int(ctx, ret);\n\treturn 1;\n}\n\n');
  })
  ws.write('static register_bindings(duk_context *ctx){\n');
  func_list.forEach(f=>{
    ws.write('\tduk_push_c_function(ctx, _'+ f[0] +', '+ f[1] +');\n');
    ws.write('\tduk_put_global_string(ctx, "'+ f[0] +'");\n\n');
  })
  ws.write('}');
});