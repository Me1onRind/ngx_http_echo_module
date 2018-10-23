#include "ngx_http_echo_module.h"
#include <stdlib.h>

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
        ngx_string("echo"), // 指令名为echo
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1, // 位置在localtion, 1个参数
        ngx_http_echo_nginx, // 设置解析函数
        NGX_HTTP_LOC_CONF_OFFSET, // 指定使用的内存池
        offsetof(ngx_http_echo_loc_conf_t, pre_str),  // 保存在配置结构中的偏移量
        NULL
    }
};

// 模块定义
ngx_module_t ngx_http_echo_module = {
    NGX_MODULE_V1, // 初始化前面的无需设置字段
    &ngx_http_echo_module_ctx, // http模块上下文
    ngx_http_echo_commands, // 指令集
    NGX_HTTP_MODULE, // 说明这是一个HTTP模块
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING // 初始化后面无需设置的字段
};

// 配置struct的创建函数
static void* ngx_http_echo_create_loc_conf(ngx_conf_t* cf) {
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
static char* ngx_http_echo_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child) {
    ngx_http_echo_loc_conf_t* prev = parent;
    ngx_http_echo_loc_conf_t* conf = child;
    // 如果location模块未定义 使用上层模块定义的echo值
    // 如果上层模块也未定义 使用"Nginx"
    ngx_conf_merge_str_value(conf->pre_str, prev->pre_str, "Nginx:");
    return NGX_CONF_OK;
}

// 解析指令
static char* ngx_http_echo_nginx(ngx_conf_t* cf, ngx_command_t* cmd, void* conf) {
    ngx_http_core_loc_conf_t* clcf;
    // 获取ngx_http_core_module模块的配置指针
    // ngx_http_core_module是核心模块 负责处理http请求
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    // 将自己的处理函数挂载到核心模块上
    // 理论上同一个location下只有一个handler有效,但也能让多个handler处理
    clcf->handler = ngx_http_echo_handler;
    // 把配置文件的内容写到strcut中
    ngx_conf_set_str_slot(cf, cmd, conf);
    return NGX_OK;
}

// 应答客户端的处理函数
static ngx_int_t ngx_http_echo_handler(ngx_http_request_t* r) {
    ngx_int_t rc;
    // 因为需要获取请求体 设置回调函数异步执行
    rc = ngx_http_read_client_request_body(r, ngx_http_echo_real_handler);
    // 返回值如果是300以上的 有异常发生 直接返回函数返回值
    if (rc > NGX_HTTP_SPECIAL_RESPONSE) {
        return rc;
    }
    // 表示该请求已被正确处理 但是被挂起直到下一个事件到来
    return NGX_DONE;
}

// 真正发送响应头 响应正文的处理函数
static void ngx_http_echo_real_handler(ngx_http_request_t* r) {
    ngx_chain_t * bufs;
    ngx_buf_t* b;
    ngx_chain_t out;
    ngx_http_echo_loc_conf_t* cf;
    char* r_body;
    size_t len;

    // 获取保存配置结构的指针
    cf = ngx_http_get_module_loc_conf(r, ngx_http_echo_module);
    // 响应报文长度 = 请求体报文长度 + 前缀字符串长度
    len = atoi((const char*)r->headers_in.content_length->value.data) + cf->pre_str.len;
    // 设置content_lenth字段
    r->headers_out.content_length_n = len;
    // 申请响应正文内存
    r_body = ngx_pcalloc(r->pool, len);
    if (r_body == NULL) {

    }

    // 先把前缀字符串写进去
    len = cf->pre_str.len;
    ngx_memcpy(r_body, cf->pre_str.data, len);
    // 把请求正文写进去
    bufs = r->request_body->bufs;
    while (bufs) {
        b = bufs->buf;
        ngx_memcpy(r_body + len, b->pos, b->last - b->pos);
        len += b->last - b->pos;
        bufs = bufs->next;
    }

    // 申请buf 并设置数据
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {

    }
    b->pos = (u_char*)r_body;
    b->last = b->pos + len;
    b->memory = 1;
    b->last_buf = 1;

    // 设置响应正文
    out.buf = b;
    out.next = NULL;

    // 设置请求头
    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char*)"text/plain";
    r->headers_out.status = NGX_HTTP_OK;

    ngx_http_send_header(r);
    ngx_http_output_filter(r, &out);

}

