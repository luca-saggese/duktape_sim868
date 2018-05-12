duk_ret_t _eat_network_get_creg(duk_context *ctx) {
	EatCregState_enum ret = eat_network_get_creg();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_network_get_ber(duk_context *ctx) {
	s32 ret = eat_network_get_ber();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _eat_network_get_csq(duk_context *ctx) {
	s32 ret = eat_network_get_csq();
	duk_push_int(ctx, ret);
	return 1;
}

duk_ret_t _at_network_get_cgatt(duk_context *ctx) {
	u8 ret = at_network_get_cgatt();
	duk_push_int(ctx, ret);
	return 1;
}

static register_bindings(duk_context *ctx){
	duk_push_c_function(ctx, _eat_network_get_creg, 0);
	duk_put_global_string(ctx, "eat_network_get_creg");

	duk_push_c_function(ctx, _eat_network_get_ber, 0);
	duk_put_global_string(ctx, "eat_network_get_ber");

	duk_push_c_function(ctx, _eat_network_get_csq, 0);
	duk_put_global_string(ctx, "eat_network_get_csq");

	duk_push_c_function(ctx, _at_network_get_cgatt, 0);
	duk_put_global_string(ctx, "at_network_get_cgatt");

}