#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_str_t  pre_str;
} ngx_http_echo_loc_conf_t;

/**
 * 创建保存配置的struct
 * cf 保存从配置文件读取到的原始字符串以及相关的一些信息
 */
static void* ngx_http_echo_create_loc_conf(ngx_conf_t* cf);
/**
 * 解析指令
 * cf 保存从配置文件读取到的原始字符串以及相关的一些信息
 * parent 上层的配置struct指针
 * child 本层配置struct指针
 */
static char* ngx_http_echo_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child);
/**
 * 解析指令
 * cf 保存从配置文件读取到的原始字符串以及相关的一些信息
 * cmd 指令对应的ngx_command_t
 * conf 存储配置的struct 这里即ngx_http_echo_loc_conf_t
 */
static char* ngx_http_echo_nginx(ngx_conf_t* cf, ngx_command_t* cmd, void* conf);
/**
 * r http请求的strcut
 */
static ngx_int_t ngx_http_echo_handler(ngx_http_request_t* r);
/**
 * 真正发送响应头和响应体
 * 使用这个函数是因为解析请求头是同步过程, 解析请求体是异步
 * 所以需要实现一个回调函数
 * r http请求的strcut
 */
static void ngx_http_echo_real_handler(ngx_http_request_t* r);
