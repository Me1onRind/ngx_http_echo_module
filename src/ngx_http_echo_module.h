#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_str_t pre_str;
} ngx_http_echo_loc_conf_t;

// 创建配置struct
static void* ngx_http_echo_create_loc_conf(ngx_conf_t* cf);
// 合并main srv loc级别配置到srv级别配置中
static char* ngx_http_echo_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child);
// 指令执行的函数
static char* ngx_http_echo_nginx(ngx_conf_t* cf, ngx_command_t* cmd, void* conf);
// 设置应答客户端的回调函数
static ngx_int_t ngx_http_echo_handler(ngx_http_request_t* r);
// 异步发送请求体
static void ngx_http_echo_real_handler(ngx_http_request_t* r);

