#include "ngx_http_echo_module.h"
#include <string.h>

// 模块上下文
static ngx_http_module_t ngx_http_echo_module_ctx = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ngx_http_echo_create_loc_conf,
    ngx_http_echo_merge_loc_conf,
};

static ngx_command_t ngx_http_echo_commands[] = {
    {
        ngx_string("echo"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_http_echo_nginx,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_echo_loc_conf_t, pre_str),
        NULL
    },
    ngx_null_command
};

// http模块上下文定义
ngx_module_t ngx_http_echo_module = {
    NGX_MODULE_V1, // 初始化前面的无需设置字段
    &ngx_http_echo_module_ctx,
    ngx_http_echo_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
};

// 配置struct的创建函数
void* ngx_http_echo_create_loc_conf(ngx_conf_t* cf) {
    ngx_http_echo_loc_conf_t* conf;
    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_echo_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    conf->pre_str.len = 0;
    conf->pre_str.data = NULL;
    return conf;
}

// 配置的合并
char* ngx_http_echo_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child) {
    ngx_http_echo_loc_conf_t* prev = parent;
    ngx_http_echo_loc_conf_t* conf = child;
    ngx_conf_merge_str_value(conf->pre_str, prev->pre_str, "Nginx");
    return NGX_CONF_OK;
}

// 指令应的函数
static char* ngx_http_echo_nginx(ngx_conf_t* cf, ngx_command_t* cmd, void* conf) {
    ngx_http_core_loc_conf_t* clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_echo_handler;
    ngx_conf_set_str_slot(cf, cmd, conf);
    return NGX_CONF_OK;
}

u_char str[] = "123";
// 指令对应的回调函数 应答客户端
static ngx_int_t ngx_http_echo_handler(ngx_http_request_t* r) {
    ngx_int_t rc;
    rc = ngx_http_read_client_request_body(r, ngx_http_echo_real_handler);
    if (rc > NGX_HTTP_SPECIAL_RESPONSE) {
        return rc;
    }
    return NGX_DONE;
}

static void ngx_http_echo_real_handler(ngx_http_request_t* r) {
    ngx_buf_t* b;
    ngx_chain_t out;
    ngx_http_echo_loc_conf_t* cf;
    char* r_body;
    int ret_len;

    cf = ngx_http_get_module_loc_conf(r, ngx_http_echo_module);

    b = r->request_body->buf;
    /*body_len = b->last - b->pos;*/
    ret_len = cf->pre_str.len + 10;
    r_body = (char*)ngx_pcalloc(r->pool, 16);
    /*if (b == NULL) {*/
        ret_len = 1;
    /*} else {*/
        /*ret_len = 2;*/
    /*}*/
    ngx_memcpy(r_body, b->pos, ret_len);
    /*memcpy(str, cf->pre_str.data, cf->pre_str.len);*/

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    b->pos = (u_char*)r_body;
    b->last = b->pos + ret_len;
    b->memory = 1;
    b->last_buf = 1;

    out.buf = b;
    out.next = NULL;

    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char*)"text/plain";
    r->headers_out.content_length_n = ret_len;
    r->headers_out.status = NGX_HTTP_OK;

    ngx_http_send_header(r);
    ngx_http_output_filter(r, &out);
}

