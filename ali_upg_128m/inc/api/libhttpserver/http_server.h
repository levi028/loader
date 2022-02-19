#ifndef HTTP_MG_SERVER_H_
#define HTTP_MG_SERVER_H_
#include <basic_types.h>

/** User define URI's max string len. */
#define HTTP_MG_MAX_URI_LEN 128
/** Max user define URI can support. */
#define HTTP_MG_MAX_METHOD_SUPPORT 64

/** HTTP connection handle. */
typedef void* HTTP_MG_HANDLE;

/**
 *  @brief      Define user define process method's return type.
 */
typedef enum 
{
    METHOD_RET_PROCESSED = 0,
    METHOD_RET_NOPROCESS,
} HTTP_MG_METHOD_RET_E;

/**
 *  @brief      Define the HTTP response code.
 */
typedef enum 
{
    HTTP_STATUS_CONTINUE = 100,
    HTTP_STATUS_SWITCHING_PROTOCOLS = 101,
    HTTP_STATUS_OK = 200,
    HTTP_STATUS_CREATED = 201,
    HTTP_STATUS_ACCEPTED = 202,
    HTTP_STATUS_NON_AUTHORITATIVE_INFORMATION = 203,
    HTTP_STATUS_NO_CONTENT = 204,
    HTTP_STATUS_RESET_CONTENT = 205,
    HTTP_STATUS_PARTIAL_CONTENT = 206,
    HTTP_STATUS_MULTIPLE_CHOICES = 300,
    HTTP_STATUS_MOVED_PERMANENTLY = 301,
    HTTP_STATUS_FOUND = 302,
    HTTP_STATUS_SEE_OTHER = 303,
    HTTP_STATUS_NOT_MODIFIED = 304,
    HTTP_STATUS_USE_PROXY = 305,
    HTTP_STATUS_TEMPORARY_REDIRECT = 307,
    HTTP_STATUS_BAD_REQUEST = 400,
    HTTP_STATUS_UNAUTHORIZED = 401,
    HTTP_STATUS_PAYMENT_REQUIRED = 402,
    HTTP_STATUS_FORBIDDEN = 403,
    HTTP_STATUS_NOT_FOUND = 404,
    HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
    HTTP_STATUS_NOT_ACCEPTABLE = 406,
    HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED = 407,
    HTTP_STATUS_REQUEST_TIME_OUT = 408,
    HTTP_STATUS_CONFLICT = 409,
    HTTP_STATUS_GONE = 410,
    HTTP_STATUS_LENGTH_REQUIRED = 411,
    HTTP_STATUS_PRECONDITION_FAILED = 412,
    HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE = 413,
    HTTP_STATUS_REQUEST_URL_TOO_LARGE = 414,
    HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE = 415,
    HTTP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
    HTTP_STATUS_EXPECTATION_FAILED = 417,
    HTTP_STATUS_INTERNAL_SERVER_ERROR = 500,
    HTTP_STATUS_NOT_IMPLEMENTED = 501,
    HTTP_STATUS_BAD_GATEWAY = 502,
    HTTP_STATUS_SERVICE_UNAVAILABLE = 503,
    HTTP_STATUS_GATEWAY_TIME_OUT = 504,
    HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED = 505,
}HTTP_RESPONSE_STATUS_CODE;

/**
 *  @brief      The structure that store the information from http client request.
 */
typedef struct 
{
    /** "GET", "POST", etc */
    char *request_method;
    /** URL-decoded URI */
    char *uri;
    /** // E.g. "1.0", "1.1" */
    char *http_version;
    /** URL part after '?' (not including '?') or NULL. */
    char *query_string;
    /** Authenticated user, or NULL if no auth used. */
    char *remote_user;
    /** Client's IP address. */
    long remote_ip;
    /** Client's port. */
    int remote_port;
    /** Number of headers. */
    int num_headers;
    /** Maximum of header support is 64. */
    struct header {
        /** HTTP header name. */
        char *name;
        /** HTTP header value. */
        char *value;
    } http_headers[64];
} http_mg_req_info;


/**
 *  @brief  http_mg_method_t prototype of http method.
 */
typedef HTTP_MG_METHOD_RET_E (*http_mg_method_t)(HTTP_MG_HANDLE h, http_mg_req_info *req_info);


/**
 *  @brief      Start ALi default http server for media.
 *  @return     TRUE if success; FALSE if failure.
 */
BOOL http_mg_server_start(void);

/**
 *  @brief      Stop ALi default http server for media.
 */
void http_mg_server_stop(void);

/**
 *  @brief      Use to send one file in http response content.
 *  @param[in]  h This connection handle.
 *  @param[in]  full_path The file's full path from root directory.
 */
void http_mg_server_send_file(HTTP_MG_HANDLE h, const char *full_path);

/**
 *  @brief      Read data from this connection, e.g. POST body data.
 *  @param[in]  h The handle of this connection.
 *  @param[in]  buf The data buffer use to store data.
 *  @param[in]  len The data buffer's length.
 *  @return     The byte count have read.
 */
INT32 http_mg_server_read(HTTP_MG_HANDLE h, UINT8 *buf, UINT32 len);

/**
 *  @brief      Send data out to client.
 *  @param[in]  h This connection handle.
 *  @param[in]  buf The data buffer going to send out.
 *  @param[in]  len The data buffer's data length.
 *  @return     The byte count have sent out.
 */
INT32 http_mg_server_write(HTTP_MG_HANDLE h, UINT8 *buf, UINT32 len);

/**
 *  @brief      Use to send out some response.
 *  @param[in]  h This connection handle.
 *  @param[in]  error_status_code response code, ref to HTTP_RESPONSE_STATUS_CODE.
 *  @return     TRUE if send success; FALSE if send failure or this response code no quick implement.
 */
BOOL http_mg_quick_response(HTTP_MG_HANDLE h, INT32 error_status_code);

/**
 *  @brief      Register method callback to http mg server with unique uri.
 *  @details    Register method callback to http mg server with unique uri,
 *              e.g. uri = "/a/b/c/test_method", method=mytest_method, when client request
 *              "http://IP:PORT/a/b/c/test_method", server will callback to function mytest_method,
 *              you can do your process in this function, and you need send out all response content
 *              (include headers) as http request response.
 *  @param[in]  uri The uri that you want to identify your method.
 *  @param[in]  method The callback function you want to be called.
 *  @return     TRUE if register success; FALSE if failure.
 */
BOOL http_mg_method_register(char *uri, http_mg_method_t method);

/**
 *  @brief      Unregister the method in the specified uri.
 *  @param[in]  uri The URI going to unregister.
 */
void http_mg_method_unregister(char *uri);

/**
 *  @brief      Check is http mg server is started.
 *  @return     TRUE if server already started; FALSE if server is not started.
 */
BOOL http_mg_server_is_started(void);

/**
 *  @brief      Get HTTP header's value from this request.
 *  @param[in]  h This connection handle.
 *  @param[in]  name Header name.
 *  @return     The header's value if success, don't free this pointer.
 */
char *http_mg_get_header(HTTP_MG_HANDLE h, const char *name);

/**
 *  @brief      Get Range header's value.
 *  @param[in]  header Range header's value string
 *  @param[out] a range begin value
 *  @param[out] b range end value
 *  @return     The parsed value count.
 */
INT32 http_mg_parse_range_header(const char *header, INT64 *a, INT64 *b);

#endif //HTTP_MG_SERVER_H_
